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
  * Scheduler abstraction of a worker.
  *
  * Author: Philip Levis <pal@cs.stanford.edu>
  * Author: Omid Mashayekhi <omidm@stanford.edu>
  */

#ifndef NIMBUS_SRC_SCHEDULER_SCHEDULER_WORKER_H_
#define NIMBUS_SRC_SCHEDULER_SCHEDULER_WORKER_H_

#include <list>
#include <string>
#include "src/shared/scheduler_server_connection.h"
#include "src/scheduler/application_group.h"

namespace nimbus {


class SchedulerWorker {
 public:
  SchedulerWorker(worker_id_t id,
                  SchedulerServerConnection* conn,
                  ApplicationGroup* app);
  virtual ~SchedulerWorker();

  virtual worker_id_t worker_id() const;
  virtual std::string ip() const;
  virtual void set_ip(std::string ip);
  virtual port_t port() const;
  virtual void set_port(port_t port);
  virtual SchedulerServerConnection* connection();
  virtual ApplicationGroup* application();
  virtual bool is_alive() const;
  virtual bool handshake_done() const;
  virtual void set_handshake_done(bool flag);
  virtual void MarkDead();

 private:
  worker_id_t worker_id_;
  std::string ip_;
  port_t port_;
  SchedulerServerConnection* connection_;
  ApplicationGroup* application_;
  bool is_alive_;
  bool handshake_done_;
};

typedef std::list<SchedulerWorker*> SchedulerWorkerList;

}  // namespace nimbus

#endif  // NIMBUS_SRC_SCHEDULER_SCHEDULER_WORKER_H_
