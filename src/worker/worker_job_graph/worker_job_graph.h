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
  * Author: Hang Qu <quhang@stanford.edu>
  */

#ifndef NIMBUS_SRC_WORKER_WORKER_JOB_GRAPH_WORKER_JOB_GRAPH_H_
#define NIMBUS_SRC_WORKER_WORKER_JOB_GRAPH_WORKER_JOB_GRAPH_H_

#include <vector>
#include <string>
#include <set>
#include <list>
#include <utility>
#include <map>
#include "src/shared/graph.h"
#include "src/shared/nimbus_types.h"
#include "src/worker/worker_job_graph/worker_job_entry.h"

namespace nimbus {

typedef Graph<WorkerJobEntry, job_id_t> WorkerJobGraph;
typedef Vertex<WorkerJobEntry, job_id_t> WorkerJobVertex;
typedef Edge<WorkerJobEntry, job_id_t> WorkerJobEdge;

}  // namespace nimbus

#endif  // NIMBUS_SRC_WORKER_WORKER_JOB_GRAPH_WORKER_JOB_GRAPH_H_
