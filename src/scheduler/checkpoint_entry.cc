/*
 * Copyright 2013 Stanford University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of the copyright holders nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

 /*
  * Scheduler Checkpoint Entry. This is the class that keeps the meta data for
  * each created checkpoint in the system.
  *
  * Author: Omid Mashayekhi <omidm@stanford.edu>
  */

#include "src/scheduler/checkpoint_entry.h"

using namespace nimbus; // NOLINT

CheckpointEntry::CheckpointEntry(checkpoint_id_t checkpoint_id) {
  checkpoint_id_ = checkpoint_id;
  pending_count_ = -1;
}

CheckpointEntry::~CheckpointEntry() {
  JobEntryMap::iterator it = jobs_.begin();
  for (; it != jobs_.end(); ++it) {
    delete it->second;
  }
}

bool CheckpointEntry::AddJob(const JobEntry *job) {
  JobEntry* j =
    new ComputeJobEntry(job->job_name(),
                        job->job_id(),
                        job->read_set(),
                        job->write_set(),
                        job->scratch_set(),
                        job->reduce_set(),
                        job->before_set(),
                        job->after_set(),
                        job->parent_job_id(),
                        job->future_job_id(),
                        job->sterile(),
                        job->region(),
                        job->params(),
                        job->combiner_map());

  jobs_[j->job_id()] = j;

  IncreasePendingCounter();
  return true;
}

bool CheckpointEntry::CompleteJob(const JobEntry *job) {
  job_id_t job_id = job->job_id();
  JobEntryMap::iterator it = jobs_.find(job_id);
  if (it == jobs_.end()) {
    dbg(DBG_ERROR, "ERROR: job with id %lu is not in jobs!\n", job_id);
    assert(false);
    return false;
  }

  assert(job->versioned_entire_context());

  it->second->set_vmap_read(job->vmap_read());
  it->second->set_vmap_write(job->vmap_write());
  it->second->set_job_depth(job->job_depth());
  it->second->MarkJobAsCompletelyResolved();

  DecreasePendingCounter();
  return true;
}

bool CheckpointEntry::AddSaveDataJob(job_id_t job_id,
                                     logical_data_id_t ldid,
                                     data_version_t version,
                                     worker_id_t worker_id) {
  map_[job_id] = LVW(ldid, version, worker_id);
  IncreasePendingCounter();
  return true;
}

bool CheckpointEntry::NotifySaveDataJobDone(job_id_t job_id,
                                            std::string handle) {
  Map::iterator it = map_.find(job_id);
  if (it == map_.end()) {
    dbg(DBG_ERROR, "ERROR: save job with id %lu is not in map!\n", job_id);
    assert(false);
    return false;
  }

  logical_data_id_t ldid = boost::get<0>(it->second);
  data_version_t version = boost::get<1>(it->second);
  worker_id_t worker_id = boost::get<2>(it->second);

  index_[ldid][version].push_back(std::make_pair(worker_id, handle));

  DecreasePendingCounter();
  return true;
}

bool CheckpointEntry::GetJobList(JobEntryList *list) {
  list->clear();
  JobEntryMap::iterator it = jobs_.begin();
  for (; it != jobs_.end(); ++it) {
    list->push_back(it->second);
  }

  return true;
}

bool CheckpointEntry::GetHandleToLoadData(logical_data_id_t ldid,
                                              data_version_t version,
                                              WorkerHandleList *handles) {
  Index::iterator iter = index_.find(ldid);
  if (iter == index_.end()) {
    dbg(DBG_ERROR, "ERROR: ldid %lu is not in index!\n", ldid);
    assert(false);
    return false;
  }

  VersionIndex::iterator it = iter->second.find(version);
  if (it == iter->second.end()) {
    dbg(DBG_ERROR, "ERROR: version %lu is not in index for ldid %lu!\n", version, ldid);
    assert(false);
    return false;
  }

  handles->clear();
  WorkerHandleList::iterator i = it->second.begin();
  for (; i != it->second.end(); ++i) {
    handles->push_back(*i);
  }

  return true;
}

bool CheckpointEntry::IsComplete() {
  return (pending_count_ == 0);
}

void CheckpointEntry::IncreasePendingCounter() {
  if (pending_count_ == -1) {
    pending_count_ = 1;
  } else {
    ++pending_count_;
  }
}

void CheckpointEntry::DecreasePendingCounter() {
    --pending_count_;
}


