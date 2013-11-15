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
  * Spawn compute job command used to spawn compute jobs from worker to
  * scheduler.
  *
  * Author: Omid Mashayekhi <omidm@stanford.edu>
  */

#ifndef NIMBUS_SHARED_SPAWN_COMPUTE_JOB_COMMAND_H_
#define NIMBUS_SHARED_SPAWN_COMPUTE_JOB_COMMAND_H_


#include <string>
#include "shared/scheduler_command.h"

namespace nimbus {
class SpawnComputeJobCommand : public SchedulerCommand {
  public:
    SpawnComputeJobCommand();
    SpawnComputeJobCommand(const std::string& job_name,
        const ID<job_id_t>& job_id,
        const IDSet<logical_data_id_t>& read, const IDSet<logical_data_id_t>& write,
        const IDSet<job_id_t>& before, const IDSet<job_id_t>& after,
        const ID<job_id_t>& parent_job_id,
        const Parameter& params);
    ~SpawnComputeJobCommand();

    virtual SchedulerCommand* Clone();
    virtual bool Parse(const std::string& param_segment);
    virtual std::string toString();
    virtual std::string toStringWTags();
    std::string job_name();
    ID<job_id_t> job_id();
    IDSet<logical_data_id_t> read_set();
    IDSet<logical_data_id_t> write_set();
    IDSet<job_id_t> before_set();
    IDSet<job_id_t> after_set();
    ID<job_id_t> parent_job_id();
    Parameter params();

  private:
    std::string job_name_;
    ID<job_id_t> job_id_;
    IDSet<logical_data_id_t> read_set_;
    IDSet<logical_data_id_t> write_set_;
    IDSet<job_id_t> before_set_;
    IDSet<job_id_t> after_set_;
    ID<job_id_t> parent_job_id_;
    Parameter params_;
};



}  // namespace nimbus

#endif  // NIMBUS_SHARED_SPAWN_COMPUTE_JOB_COMMAND_H_
