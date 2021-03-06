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
  * Job profile in the job graph of load balancer.
  *
  * Author: Omid Mashayekhi <omidm@stanford.edu>
  */

#ifndef NIMBUS_SRC_SCHEDULER_JOB_PROFILE_H_
#define NIMBUS_SRC_SCHEDULER_JOB_PROFILE_H_

#include <vector>
#include <string>
#include <set>
#include <list>
#include <utility>
#include <map>
#include "src/worker/data.h"
#include "src/shared/idset.h"
#include "src/shared/parameter.h"
#include "src/shared/nimbus_types.h"
#include "src/scheduler/version_map.h"
#include "src/scheduler/meta_before_set.h"
#include "src/scheduler/logical_data_lineage.h"

namespace nimbus {

class JobProfile;
typedef std::map<job_id_t, JobProfile*> JobProfileMap;
typedef std::map<job_id_t, JobProfile*> JobProfileTable;
typedef std::list<JobProfile*> JobProfileList;

class JobProfile {
  public:
    class LogEntry {
      public:
        LogEntry(
            worker_id_t worker_id,
            job_id_t job_id,
            std::string job_name,
            double done_time);

        virtual ~LogEntry();

        worker_id_t worker_id();
        job_id_t job_id();
        std::string job_name();
        double done_time();

        std::string ToString();

      private:
        worker_id_t worker_id_;
        job_id_t job_id_;
        std::string job_name_;
        double done_time_;
    };

    typedef std::list<LogEntry> DependencyLog;

    JobProfile();

    JobProfile(
        const JobType& job_type,
        const std::string& job_name,
        const job_id_t& job_id,
        const job_id_t& parent_job_id,
        const worker_id_t& worker_id,
        const bool& sterile);

    virtual ~JobProfile();

    JobType job_type();
    std::string job_name();
    job_id_t job_id();
    job_id_t parent_job_id();
    worker_id_t worker_id();
    bool sterile();

    bool assigned();
    bool ready();
    bool done();
    double assign_time();
    double ready_time();
    double done_time();
    double execute_duration();


    void set_job_type(JobType job_type);
    void set_job_name(std::string job_name);
    void set_job_id(job_id_t job_id);
    void set_parent_job_id(job_id_t parent_job_id);
    void set_worker_id(worker_id_t worker_id);
    void set_sterile(bool flag);

    void set_assigned(bool flag);
    void set_ready(bool flag);
    void set_done(bool flag);
    void set_assign_time(double assign_time);
    void set_ready_time(double ready_time);
    void set_done_time(double done_time);
    void set_execute_duration(double execute_duration);

    IDSet<job_id_t>* waiting_set_p();

    void add_log_entry(
        worker_id_t worker_id,
        job_id_t job_id,
        std::string job_name,
        double done_time);

    std::string PrintDependencyLog();

    std::string Print();

    bool FindBlamedWorker(worker_id_t *worker_id);

  private:
    JobType job_type_;
    std::string job_name_;
    job_id_t job_id_;
    job_id_t parent_job_id_;
    worker_id_t worker_id_;
    bool sterile_;
    IDSet<job_id_t> waiting_set_;

    DependencyLog dependency_log_;

    bool assigned_;
    bool ready_;
    bool done_;
    double assign_time_;
    double ready_time_;
    double done_time_;
    double execute_duration_;


    void Initialize();
};


}  // namespace nimbus
#endif  // NIMBUS_SRC_SCHEDULER_JOB_PROFILE_H_


