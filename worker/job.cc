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
  * A Nimbus job. 
  *
  * Author: Omid Mashayekhi <omidm@stanford.edu>
  */

#include "worker/job.h"
#include "worker/application.h"

using namespace nimbus; // NOLINT

Job::Job() {
  app_is_set_ = false;
}

Job::~Job() {
}

// TODO(omidm) should remove this later. left it now so the tests
// that use it still pass.
Job::Job(Application* app, JobType type) {
  application_ = app;
  type_ = type;
}

Job* Job::Clone() {
  std::cout << "cloning the base job\n";
  Job* j = new Job();
  return j;
}

bool Job::SpawnComputeJob(const std::string& name,
    const job_id_t& id,
    const IDSet<logical_data_id_t>& read,
    const IDSet<logical_data_id_t>& write,
    const IDSet<job_id_t>& before,
    const IDSet<job_id_t>& after,
    const Parameter& params) {
  if (app_is_set_) {
    application_->SpawnComputeJob(name, id, read, write, before, after, params);
    return true;
  } else {
    std::cout << "ERROR: SpawnComputeJob, application has not been set." <<
      std::endl;
    return false;
  }
}

bool Job::SpawnCopyJob(const job_id_t& id,
    const logical_data_id_t& from_logical_id,
    const logical_data_id_t& to_logical_id,
    const IDSet<job_id_t>& before,
    const IDSet<job_id_t>& after,
    const Parameter& params) {
  if (app_is_set_) {
    application_->SpawnCopyJob(id, from_logical_id, to_logical_id, before, after, params);
    return true;
  } else {
    std::cout << "ERROR: SpawnCopyJob, application has not been set." <<
      std::endl;
    return false;
  }
}

bool Job::DefineData(const std::string& name,
    const logical_data_id_t& logical_data_id,
    const partition_id_t& partition_id,
    const IDSet<partition_id_t>& neighbor_partition,
    const Parameter& params) {
  if (app_is_set_) {
    application_->DefineData(name, logical_data_id, partition_id, neighbor_partition, params);
    return true;
  } else {
    std::cout << "ERROR: DefineData, application has not been set." <<
      std::endl;
    return false;
  }
}

bool Job::GetNewJobID(std::vector<job_id_t>* result, size_t req_num) {
  if (app_is_set_) {
    return application_->GetNewJobID(result, req_num);
  } else {
    std::cout << "ERROR: GetNewJobID, application has not been set." <<
      std::endl;
    return false;
  }
}

bool Job::GetNewLogicalDataID(std::vector<logical_data_id_t>* result, size_t req_num) {
  if (app_is_set_) {
    return application_->GetNewLogicalDataID(result, req_num);
  } else {
    std::cout << "ERROR: GetNewDataID, application has not been set." <<
      std::endl;
    return false;
  }
}

std::string Job::name() {
  return name_;
}

ID<job_id_t> Job::id() {
  return id_;
}

IDSet<physical_data_id_t> Job::read_set() {
  return read_set_;
}

IDSet<physical_data_id_t> Job::write_set() {
  return write_set_;
}

IDSet<job_id_t> Job::before_set() {
  return before_set_;
}

IDSet<job_id_t> Job::after_set() {
  return after_set_;
}

Parameter Job::parameters() {
  return parameters_;
}

Application* Job::application() {
  return application_;
}

void Job::set_name(std::string name) {
  name_ = name;
}

void Job::set_id(ID<job_id_t> id) {
  id_ = id;
}

void Job::set_read_set(IDSet<physical_data_id_t> read_set) {
  read_set_ = read_set;
}

void Job::set_write_set(IDSet<physical_data_id_t> write_set) {
  write_set_ = write_set;
}

void Job::set_before_set(IDSet<job_id_t> before_set) {
  before_set_ = before_set;
}

void Job::set_after_set(IDSet<job_id_t> after_set) {
  after_set_ = after_set;
}

void Job::set_parameters(Parameter parameters) {
  parameters_ = parameters;
}

void Job::set_application(Application* app) {
  application_ = app;
  app_is_set_ = true;
}




RemoteCopySendJob::RemoteCopySendJob(WorkerDataExchanger* da) {
  data_exchanger_ = da;
}

RemoteCopySendJob::~RemoteCopySendJob() {
}

void RemoteCopySendJob::Execute(Parameter params, const DataArray& da) {
  SerializedData ser_data;
  da[0]->Serialize(&ser_data);
  data_exchanger_->SendSerializedData(receive_job_id().elem(), to_worker_id_.elem(), ser_data);
  // delete ser_data.data_ptr(); // Not needed with shared pointer.
}

Job* RemoteCopySendJob::Clone() {
  return new RemoteCopySendJob(data_exchanger_);
}

ID<job_id_t> RemoteCopySendJob::receive_job_id() {
  return receive_job_id_;
}

ID<worker_id_t> RemoteCopySendJob::to_worker_id() {
  return to_worker_id_;
}

std::string RemoteCopySendJob::to_ip() {
  return to_ip_;
}

ID<port_t> RemoteCopySendJob::to_port() {
  return to_port_;
}


void RemoteCopySendJob::set_receive_job_id(ID<job_id_t> receive_job_id) {
  receive_job_id_ = receive_job_id;
}

void RemoteCopySendJob::set_to_worker_id(ID<worker_id_t> worker_id) {
  to_worker_id_ = worker_id;
}

void RemoteCopySendJob::set_to_ip(std::string ip) {
  to_ip_ = ip;
}

void RemoteCopySendJob::set_to_port(ID<port_t> port) {
  to_port_ = port;
}



RemoteCopyReceiveJob::RemoteCopyReceiveJob() {
}

RemoteCopyReceiveJob::~RemoteCopyReceiveJob() {
}

void RemoteCopyReceiveJob::Execute(Parameter params, const DataArray& da) {
  Data * data_copy;
  da[0]->DeSerialize(*serialized_data_, &data_copy);
  da[0]->Copy(data_copy);

  data_copy->Destroy();
  // delete serialized_data_->data_ptr(); // Not needed with shared pointer.
  delete serialized_data_;
}

Job* RemoteCopyReceiveJob::Clone() {
  return new RemoteCopyReceiveJob();
}

void RemoteCopyReceiveJob::set_serialized_data(SerializedData* ser_data) {
  serialized_data_ = ser_data;
}



LocalCopyJob::LocalCopyJob() {
}

LocalCopyJob::~LocalCopyJob() {
}

Job* LocalCopyJob::Clone() {
  return new LocalCopyJob();
}

void LocalCopyJob::Execute(Parameter params, const DataArray& da) {
  da[1]->Copy(da[0]);
}



CreateDataJob::CreateDataJob() {
}

CreateDataJob::~CreateDataJob() {
}

Job* CreateDataJob::Clone() {
  return new CreateDataJob();
}

void CreateDataJob::Execute(Parameter params, const DataArray& da) {
  da[0]->Create();
}



