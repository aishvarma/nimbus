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
  * This is CommandTemplate module to hold and instantiate a set of commands
  * sent from controller to worker.
  *
  * Author: Omid Mashayekhi <omidm@stanford.edu>
  */

#include "src/shared/command_template.h"
#include "src/shared/scheduler_client.h"

using namespace nimbus; // NOLINT

CommandTemplate::CommandTemplate(const std::string& command_template_name,
                                 const std::vector<job_id_t>& inner_job_ids,
                                 const std::vector<job_id_t>& outer_job_ids,
                                 const std::vector<physical_data_id_t>& phy_ids) {
  finalized_ = false;
  compute_job_num_ = 0;
  copy_job_num_ = 0;
  command_template_name_ = command_template_name;
  future_job_id_ptr_ = JobIdPtr(new job_id_t(0));

  {
    std::vector<job_id_t>::const_iterator iter = inner_job_ids.begin();
    for (; iter != inner_job_ids.end(); ++iter) {
      JobIdPtr job_id_ptr = JobIdPtr(new job_id_t(*iter));
      inner_job_id_map_[*iter] = job_id_ptr;
      inner_job_id_list_.push_back(job_id_ptr);
    }
  }

  {
    std::vector<job_id_t>::const_iterator iter = outer_job_ids.begin();
    for (; iter != outer_job_ids.end(); ++iter) {
      JobIdPtr job_id_ptr = JobIdPtr(new job_id_t(*iter));
      outer_job_id_map_[*iter] = job_id_ptr;
      outer_job_id_list_.push_back(job_id_ptr);
    }
  }

  {
    std::vector<physical_data_id_t>::const_iterator iter = phy_ids.begin();
    for (; iter != phy_ids.end(); ++iter) {
      PhyIdPtr phy_id_ptr = PhyIdPtr(new physical_data_id_t(*iter));
      phy_id_map_[*iter] = phy_id_ptr;
      phy_id_list_.push_back(phy_id_ptr);
    }
  }
}

CommandTemplate::~CommandTemplate() {
}

bool CommandTemplate::finalized() {
  boost::unique_lock<boost::mutex> lock(mutex_);
  return finalized_;
}

size_t CommandTemplate::copy_job_num() {
  boost::unique_lock<boost::mutex> lock(mutex_);
  assert(finalized_);
  return copy_job_num_;
}

size_t CommandTemplate::compute_job_num() {
  boost::unique_lock<boost::mutex> lock(mutex_);
  assert(finalized_);
  return compute_job_num_;
}

std::string CommandTemplate::command_template_name() {
  boost::unique_lock<boost::mutex> lock(mutex_);
  return command_template_name_;
}

bool CommandTemplate::Finalize() {
  boost::unique_lock<boost::mutex> lock(mutex_);
  assert(!finalized_);
  finalized_ = true;
  return true;
}

bool CommandTemplate::Instantiate(const std::vector<job_id_t>& inner_job_ids,
                                  const std::vector<job_id_t>& outer_job_ids,
                                  const IDSet<job_id_t>& extra_dependency,
                                  const std::vector<Parameter>& parameters,
                                  const std::vector<physical_data_id_t>& physical_ids,
                                  SchedulerClient *client) {
  boost::unique_lock<boost::mutex> lock(mutex_);
  assert(finalized_);
  assert(inner_job_ids.size() == inner_job_id_list_.size());
  assert(outer_job_ids.size() == outer_job_id_list_.size());
  assert(physical_ids.size() == phy_id_list_.size());

  {
    size_t idx = 0;
    JobIdPtrList::iterator iter = inner_job_id_list_.begin();
    for (; iter != inner_job_id_list_.end(); ++iter) {
      *(*iter) = inner_job_ids[idx];
      ++idx;
    }
  }

  {
    size_t idx = 0;
    JobIdPtrList::iterator iter = outer_job_id_list_.begin();
    for (; iter != outer_job_id_list_.end(); ++iter) {
      *(*iter) = outer_job_ids[idx];
      ++idx;
    }
  }

  {
    size_t idx = 0;
    PhyIdPtrList::iterator iter = phy_id_list_.begin();
    for (; iter != phy_id_list_.end(); ++iter) {
      *(*iter) = physical_ids[idx];
      ++idx;
    }
  }

  ComputeJobCommandTemplate *cc;
  CommandTemplateVector::iterator iter = command_templates_.begin();
  for (; iter != command_templates_.end(); ++iter) {
    BaseCommandTemplate *ct = *iter;
    switch (ct->type_) {
      case COMPUTE:
        cc = reinterpret_cast<ComputeJobCommandTemplate*>(ct);
        assert(cc->param_index_ < parameters.size());
        PushComputeJobCommand(cc,
                              parameters[cc->param_index_],
                              extra_dependency,
                              client);
        break;
      case COMBINE:
        PushCombineJobCommand(reinterpret_cast<CombineJobCommandTemplate*>(ct),
                              extra_dependency,
                              client);
        break;
      case LC:
        PushLocalCopyCommand(reinterpret_cast<LocalCopyCommandTemplate*>(ct),
                             extra_dependency,
                             client);
        break;
      case RCS:
        PushRemoteCopySendCommand(reinterpret_cast<RemoteCopySendCommandTemplate*>(ct),
                                  extra_dependency,
                                  client);
        break;
      case RCR:
        PushRemoteCopyReceiveCommand(reinterpret_cast<RemoteCopyReceiveCommandTemplate*>(ct),
                                     extra_dependency,
                                     client);
        break;
      case MEGA_RCR:
        PushMegaRCRCommand(reinterpret_cast<MegaRCRCommandTemplate*>(ct),
                           extra_dependency,
                           client);
        break;
      default:
        assert(false);
    }
  }

  return true;
}


void CommandTemplate::PushComputeJobCommand(ComputeJobCommandTemplate* command,
                                            const Parameter& parameter,
                                            const IDSet<job_id_t>& extra_dependency,
                                            SchedulerClient *client) {
  std::string job_name = command->job_name_;
  ID<job_id_t> job_id(*(command->job_id_ptr_));
  ID<job_id_t> future_job_id(*(command->future_job_id_ptr_));

  IDSet<physical_data_id_t> read_set, write_set, scratch_set, reduce_set;
  IDSet<job_id_t> before_set, after_set;

  {
    JobIdPtrSet::iterator it = command->before_set_ptr_.begin();
    for (; it != command->before_set_ptr_.end(); ++it) {
      before_set.insert(*(*it));
    }
  }
  {
    JobIdPtrSet::iterator it = command->after_set_ptr_.begin();
    for (; it != command->after_set_ptr_.end(); ++it) {
      after_set.insert(*(*it));
    }
  }
  {
    PhyIdPtrSet::iterator it = command->read_set_ptr_.begin();
    for (; it != command->read_set_ptr_.end(); ++it) {
      read_set.insert(*(*it));
    }
  }
  {
    PhyIdPtrSet::iterator it = command->write_set_ptr_.begin();
    for (; it != command->write_set_ptr_.end(); ++it) {
      write_set.insert(*(*it));
    }
  }
  {
    PhyIdPtrSet::iterator it = command->scratch_set_ptr_.begin();
    for (; it != command->scratch_set_ptr_.end(); ++it) {
      scratch_set.insert(*(*it));
    }
  }
  {
    PhyIdPtrSet::iterator it = command->reduce_set_ptr_.begin();
    for (; it != command->reduce_set_ptr_.end(); ++it) {
      reduce_set.insert(*(*it));
    }
  }


  ComputeJobCommand *cm =
    new ComputeJobCommand(job_name,
                          job_id,
                          read_set,
                          write_set,
                          scratch_set,
                          reduce_set,
                          before_set,
                          extra_dependency,
                          after_set,
                          future_job_id,
                          command->sterile_,
                          command->region_,
                          parameter);

  client->PushCommandToTheQueue(cm);
}

void CommandTemplate::PushCombineJobCommand(CombineJobCommandTemplate* command,
                                            const IDSet<job_id_t>& extra_dependency,
                                            SchedulerClient *client) {
  std::string job_name = command->job_name_;
  ID<job_id_t> job_id(*(command->job_id_ptr_));

  IDSet<physical_data_id_t> scratch_set, reduce_set;
  IDSet<job_id_t> before_set;

  {
    JobIdPtrSet::iterator it = command->before_set_ptr_.begin();
    for (; it != command->before_set_ptr_.end(); ++it) {
      before_set.insert(*(*it));
    }
  }
  {
    PhyIdPtrSet::iterator it = command->scratch_set_ptr_.begin();
    for (; it != command->scratch_set_ptr_.end(); ++it) {
      scratch_set.insert(*(*it));
    }
  }
  {
    PhyIdPtrSet::iterator it = command->reduce_set_ptr_.begin();
    for (; it != command->reduce_set_ptr_.end(); ++it) {
      reduce_set.insert(*(*it));
    }
  }


  CombineJobCommand *cm =
    new CombineJobCommand(job_name,
                          job_id,
                          scratch_set,
                          reduce_set,
                          before_set,
                          extra_dependency,
                          command->region_);

  client->PushCommandToTheQueue(cm);
}

void CommandTemplate::PushLocalCopyCommand(LocalCopyCommandTemplate* command,
                                           const IDSet<job_id_t>& extra_dependency,
                                           SchedulerClient *client) {
  ID<job_id_t> job_id(*(command->job_id_ptr_));
  ID<physical_data_id_t> from_data_id(*(command->from_physical_data_id_ptr_));
  ID<physical_data_id_t> to_data_id(*(command->to_physical_data_id_ptr_));

  IDSet<job_id_t> before_set;

  {
    JobIdPtrSet::iterator it = command->before_set_ptr_.begin();
    for (; it != command->before_set_ptr_.end(); ++it) {
      before_set.insert(*(*it));
    }
  }

  LocalCopyCommand *cm =
    new LocalCopyCommand(job_id,
                        from_data_id,
                        to_data_id,
                        before_set,
                        extra_dependency);

  client->PushCommandToTheQueue(cm);
}

void CommandTemplate::PushRemoteCopySendCommand(RemoteCopySendCommandTemplate* command,
                                                const IDSet<job_id_t>& extra_dependency,
                                                SchedulerClient *client) {
  ID<job_id_t> job_id(*(command->job_id_ptr_));
  ID<job_id_t> receive_job_id(*(command->receive_job_id_ptr_));
  ID<job_id_t> mega_rcr_job_id(*(command->mega_rcr_job_id_ptr_));
  ID<physical_data_id_t> from_data_id(*(command->from_physical_data_id_ptr_));
  ID<worker_id_t> to_worker_id(command->to_worker_id_);
  std::string to_ip = command->to_ip_;
  ID<port_t> to_port(command->to_port_);

  IDSet<job_id_t> before_set;

  {
    JobIdPtrSet::iterator it = command->before_set_ptr_.begin();
    for (; it != command->before_set_ptr_.end(); ++it) {
      before_set.insert(*(*it));
    }
  }

  RemoteCopySendCommand *cm =
    new RemoteCopySendCommand(job_id,
                             receive_job_id,
                             mega_rcr_job_id,
                             from_data_id,
                             to_worker_id,
                             to_ip,
                             to_port,
                             before_set,
                             extra_dependency);

  client->PushCommandToTheQueue(cm);
}

void CommandTemplate::PushRemoteCopyReceiveCommand(RemoteCopyReceiveCommandTemplate* command,
                                                   const IDSet<job_id_t>& extra_dependency,
                                                   SchedulerClient *client) {
  ID<job_id_t> job_id(*(command->job_id_ptr_));
  ID<physical_data_id_t> to_data_id(*(command->to_physical_data_id_ptr_));

  IDSet<job_id_t> before_set;

  {
    JobIdPtrSet::iterator it = command->before_set_ptr_.begin();
    for (; it != command->before_set_ptr_.end(); ++it) {
      before_set.insert(*(*it));
    }
  }

  RemoteCopyReceiveCommand *cm =
    new RemoteCopyReceiveCommand(job_id,
                                to_data_id,
                                before_set,
                                extra_dependency);

  client->PushCommandToTheQueue(cm);
}

void CommandTemplate::PushMegaRCRCommand(MegaRCRCommandTemplate* command,
                                         const IDSet<job_id_t>& extra_dependency,
                                         SchedulerClient *client) {
  ID<job_id_t> job_id(*(command->job_id_ptr_));

  std::vector<job_id_t> receive_job_ids;
  {
    JobIdPtrList::iterator it = command->receive_job_id_ptrs_.begin();
    for (; it != command->receive_job_id_ptrs_.end(); ++it) {
      receive_job_ids.push_back(*(*it));
    }
  }

  std::vector<physical_data_id_t> to_phy_ids;
  {
    PhyIdPtrList::iterator it = command->to_phy_id_ptrs_.begin();
    for (; it != command->to_phy_id_ptrs_.end(); ++it) {
      to_phy_ids.push_back(*(*it));
    }
  }

  MegaRCRCommand *cm =
    new MegaRCRCommand(job_id,
                       receive_job_ids,
                       to_phy_ids,
                       extra_dependency);

  client->PushCommandToTheQueue(cm);
}




bool CommandTemplate::AddComputeJobCommand(ComputeJobCommand* command) {
  boost::unique_lock<boost::mutex> lock(mutex_);
  assert(!finalized_);
  JobIdPtr compute_job_id_ptr = GetExistingInnerJobIdPtr(command->job_id().elem());

  PhyIdPtrSet read_set;
  {
    IDSet<physical_data_id_t>::IDSetIter iter = command->read_set_p()->begin();
    for (; iter != command->read_set_p()->end(); ++iter) {
      PhyIdPtr phy_id_ptr = GetExistingPhyIdPtr(*iter);
      read_set.insert(phy_id_ptr);
    }
  }

  PhyIdPtrSet write_set;
  {
    IDSet<physical_data_id_t>::IDSetIter iter = command->write_set_p()->begin();
    for (; iter != command->write_set_p()->end(); ++iter) {
      PhyIdPtr phy_id_ptr = GetExistingPhyIdPtr(*iter);
      write_set.insert(phy_id_ptr);
    }
  }

  PhyIdPtrSet scratch_set;
  {
    IDSet<physical_data_id_t>::IDSetIter iter = command->scratch_set_p()->begin();
    for (; iter != command->scratch_set_p()->end(); ++iter) {
      PhyIdPtr phy_id_ptr = GetExistingPhyIdPtr(*iter);
      scratch_set.insert(phy_id_ptr);
    }
  }

  PhyIdPtrSet reduce_set;
  {
    IDSet<physical_data_id_t>::IDSetIter iter = command->reduce_set_p()->begin();
    for (; iter != command->reduce_set_p()->end(); ++iter) {
      PhyIdPtr phy_id_ptr = GetExistingPhyIdPtr(*iter);
      reduce_set.insert(phy_id_ptr);
    }
  }

  JobIdPtrSet before_set;
  {
    IDSet<job_id_t>::IDSetIter iter = command->before_set_p()->begin();
    for (; iter != command->before_set_p()->end(); ++iter) {
      JobIdPtr job_id_ptr = GetExistingInnerJobIdPtr(*iter);
      before_set.insert(job_id_ptr);
    }
  }

  JobIdPtrSet after_set;
  {
    IDSet<job_id_t>::IDSetIter iter = command->after_set_p()->begin();
    for (; iter != command->after_set_p()->end(); ++iter) {
      JobIdPtr job_id_ptr = GetExistingInnerJobIdPtr(*iter);
      after_set.insert(job_id_ptr);
    }
  }

  ComputeJobCommandTemplate *cm =
    new ComputeJobCommandTemplate(command->job_name(),
                                  compute_job_id_ptr,
                                  read_set,
                                  write_set,
                                  scratch_set,
                                  reduce_set,
                                  before_set,
                                  after_set,
                                  future_job_id_ptr_,
                                  command->sterile(),
                                  command->region(),
                                  0);

  // Keep this mapping to set the param_index in Finalize - omidm
  cm->param_index_ = compute_job_num_;
  ++compute_job_num_;

  command_templates_.push_back(cm);

  return true;
}

bool CommandTemplate::AddCombineJobCommand(CombineJobCommand* command) {
  boost::unique_lock<boost::mutex> lock(mutex_);
  assert(!finalized_);
  JobIdPtr combine_job_id_ptr = GetExistingInnerJobIdPtr(command->job_id().elem());

  PhyIdPtrSet scratch_set;
  {
    IDSet<physical_data_id_t>::IDSetIter iter = command->scratch_set_p()->begin();
    for (; iter != command->scratch_set_p()->end(); ++iter) {
      PhyIdPtr phy_id_ptr = GetExistingPhyIdPtr(*iter);
      scratch_set.insert(phy_id_ptr);
    }
  }

  PhyIdPtrSet reduce_set;
  {
    IDSet<physical_data_id_t>::IDSetIter iter = command->reduce_set_p()->begin();
    for (; iter != command->reduce_set_p()->end(); ++iter) {
      PhyIdPtr phy_id_ptr = GetExistingPhyIdPtr(*iter);
      reduce_set.insert(phy_id_ptr);
    }
  }

  JobIdPtrSet before_set;
  {
    IDSet<job_id_t>::IDSetIter iter = command->before_set_p()->begin();
    for (; iter != command->before_set_p()->end(); ++iter) {
      JobIdPtr job_id_ptr = GetExistingInnerJobIdPtr(*iter);
      before_set.insert(job_id_ptr);
    }
  }

  CombineJobCommandTemplate *cm =
    new CombineJobCommandTemplate(command->job_name(),
                                  combine_job_id_ptr,
                                  scratch_set,
                                  reduce_set,
                                  before_set,
                                  command->region(),
                                  0);

  ++copy_job_num_;
  command_templates_.push_back(cm);

  return true;
}







bool CommandTemplate::AddLocalCopyCommand(LocalCopyCommand* command) {
  boost::unique_lock<boost::mutex> lock(mutex_);
  assert(!finalized_);
  JobIdPtr copy_job_id_ptr = GetExistingInnerJobIdPtr(command->job_id().elem());

  PhyIdPtr from_physical_data_id_ptr =
    GetExistingPhyIdPtr(command->from_physical_data_id().elem());

  PhyIdPtr to_physical_data_id_ptr =
    GetExistingPhyIdPtr(command->to_physical_data_id().elem());

  JobIdPtrSet before_set;
  {
    IDSet<job_id_t>::IDSetIter iter = command->before_set_p()->begin();
    for (; iter != command->before_set_p()->end(); ++iter) {
      JobIdPtr job_id_ptr = GetExistingInnerJobIdPtr(*iter);
      before_set.insert(job_id_ptr);
    }
  }

  LocalCopyCommandTemplate *cm =
    new LocalCopyCommandTemplate(copy_job_id_ptr,
                                 from_physical_data_id_ptr,
                                 to_physical_data_id_ptr,
                                 before_set,
                                 0);

  ++copy_job_num_;
  command_templates_.push_back(cm);

  return true;
}

bool CommandTemplate::AddRemoteCopySendCommand(RemoteCopySendCommand* command) {
  boost::unique_lock<boost::mutex> lock(mutex_);
  assert(!finalized_);
  JobIdPtr copy_job_id_ptr = GetExistingInnerJobIdPtr(command->job_id().elem());

  JobIdPtr receive_job_id_ptr = GetExistingInnerJobIdPtr(command->receive_job_id().elem());

  static const JobIdPtr default_mega_rcr_job_id_ptr =
    JobIdPtr(new job_id_t(NIMBUS_KERNEL_JOB_ID));

  JobIdPtr mega_rcr_job_id_ptr;
  if (command->mega_rcr_job_id().elem() == NIMBUS_KERNEL_JOB_ID) {
    mega_rcr_job_id_ptr = default_mega_rcr_job_id_ptr;
  } else {
    mega_rcr_job_id_ptr = GetExistingInnerJobIdPtr(command->mega_rcr_job_id().elem());
  }

  PhyIdPtr from_physical_data_id_ptr =
    GetExistingPhyIdPtr(command->from_physical_data_id().elem());

  JobIdPtrSet before_set;
  {
    IDSet<job_id_t>::IDSetIter iter = command->before_set_p()->begin();
    for (; iter != command->before_set_p()->end(); ++iter) {
      JobIdPtr job_id_ptr = GetExistingInnerJobIdPtr(*iter);
      before_set.insert(job_id_ptr);
    }
  }

  RemoteCopySendCommandTemplate *cm =
    new RemoteCopySendCommandTemplate(copy_job_id_ptr,
                                      receive_job_id_ptr,
                                      mega_rcr_job_id_ptr,
                                      from_physical_data_id_ptr,
                                      command->to_worker_id(),
                                      command->to_ip(),
                                      command->to_port(),
                                      before_set,
                                      0);

  ++copy_job_num_;
  command_templates_.push_back(cm);

  return true;
}

bool CommandTemplate::AddRemoteCopyReceiveCommand(RemoteCopyReceiveCommand* command) {
  boost::unique_lock<boost::mutex> lock(mutex_);
  assert(!finalized_);
  JobIdPtr copy_job_id_ptr = GetExistingInnerJobIdPtr(command->job_id().elem());

  PhyIdPtr to_physical_data_id_ptr =
    GetExistingPhyIdPtr(command->to_physical_data_id().elem());

  JobIdPtrSet before_set;
  {
    IDSet<job_id_t>::IDSetIter iter = command->before_set_p()->begin();
    for (; iter != command->before_set_p()->end(); ++iter) {
      JobIdPtr job_id_ptr = GetExistingInnerJobIdPtr(*iter);
      before_set.insert(job_id_ptr);
    }
  }

  RemoteCopyReceiveCommandTemplate *cm =
    new RemoteCopyReceiveCommandTemplate(copy_job_id_ptr,
                                         to_physical_data_id_ptr,
                                         before_set,
                                         0);

  ++copy_job_num_;
  command_templates_.push_back(cm);

  return true;
}

bool CommandTemplate::AddMegaRCRCommand(MegaRCRCommand* command) {
  boost::unique_lock<boost::mutex> lock(mutex_);
  assert(!finalized_);
  JobIdPtr copy_job_id_ptr = GetExistingInnerJobIdPtr(command->job_id().elem());

  JobIdPtrList receive_job_id_ptrs;
  {
    std::vector<job_id_t>::const_iterator iter = command->receive_job_ids_p()->begin();
    for (; iter != command->receive_job_ids_p()->end(); ++iter) {
      JobIdPtr job_id_ptr = GetExistingInnerJobIdPtr(*iter);
      receive_job_id_ptrs.push_back(job_id_ptr);
    }
  }

  PhyIdPtrList to_phy_id_ptrs;
  {
    std::vector<physical_data_id_t>::const_iterator iter =
      command->to_physical_data_ids_p()->begin();
    for (; iter != command->to_physical_data_ids_p()->end(); ++iter) {
      PhyIdPtr phy_id_ptr = GetExistingPhyIdPtr(*iter);
      to_phy_id_ptrs.push_back(phy_id_ptr);
    }
  }


  MegaRCRCommandTemplate *cm =
    new MegaRCRCommandTemplate(copy_job_id_ptr,
                               receive_job_id_ptrs,
                               to_phy_id_ptrs,
                               0);

  copy_job_num_ += command->receive_job_ids_p()->size();
  command_templates_.push_back(cm);

  return true;
}

CommandTemplate::JobIdPtr CommandTemplate::GetExistingInnerJobIdPtr(job_id_t job_id) {
  JobIdPtr job_id_ptr;

  JobIdPtrMap::iterator iter = inner_job_id_map_.find(job_id);
  if (iter != inner_job_id_map_.end()) {
    job_id_ptr = iter->second;
  } else {
    assert(false);
  }

  return job_id_ptr;
}

CommandTemplate::PhyIdPtr CommandTemplate::GetExistingPhyIdPtr(physical_data_id_t pdid) {
  PhyIdPtr phy_id_ptr;

  PhyIdPtrMap::iterator iter = phy_id_map_.find(pdid);
  if (iter != phy_id_map_.end()) {
    phy_id_ptr = iter->second;
  } else {
    assert(false);
  }

  return phy_id_ptr;
}


