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
  * Server side of the Nimbus scheduler protocol. 
  *
  * Author: Omid Mashayekhi <omidm@stanford.edu>
  */


#ifndef NIMBUS_LIB_SCHEDULER_SERVER_H_
#define NIMBUS_LIB_SCHEDULER_SERVER_H_

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <string>
#include <map>
#include "lib/parser.h"
#include "lib/scheduler_command.h"

class Scheduler;
class SchedulerServerConnection;
typedef unsigned int ConnectionId;
typedef std::map<ConnectionId, SchedulerServerConnection*> ConnectionMap;
typedef ConnectionMap::iterator ConnectionMapIter;

using boost::asio::ip::tcp;

class SchedulerServer {
  public:
    explicit SchedulerServer(ConnectionId port_no);
    ~SchedulerServer();

    ConnectionMap connections;

    void run();

    SchedulerCommand* receiveCommand(SchedulerServerConnection* conn);

    void sendCommand(SchedulerServerConnection* conn, SchedulerCommand* c);

  private:
    boost::mutex map_mutex;

    boost::thread* connection_subscription_thread;

    ConnectionId connection_port_no;

    boost::asio::io_service* io_service;

    Scheduler * scheduler;

    void listenForNewConnections();
};


class SchedulerServerConnection {
  public:
    explicit SchedulerServerConnection(tcp::socket* sock);
    ~SchedulerServerConnection();
    ConnectionId get_id() const {
        return id;
    }

    tcp::socket* socket;

  private:
    ConnectionId id;
};

#endif  // NIMBUS_LIB_SCHEDULER_SERVER_H_