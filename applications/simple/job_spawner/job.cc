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
 * Job classes for job spawner application.
 *
 * Author: Omid Mashayekhi<omidm@stanford.edu>
 */

#include "./job.h"
#include "./data.h"
#include "./utils.h"



#define LOOP_COUNTER static_cast<JobSpawnerApp*>(application())->counter_
#define LOOP_CONDITION 0
#define STAGE_NUM static_cast<JobSpawnerApp*>(application())->stage_num_
#define JOB_LENGTH_USEC static_cast<JobSpawnerApp*>(application())->job_length_usec_
#define PART_NUM static_cast<JobSpawnerApp*>(application())->part_num_
#define CHUNK_PER_PART static_cast<JobSpawnerApp*>(application())->chunk_per_part_
#define CHUNK_SIZE static_cast<JobSpawnerApp*>(application())->chunk_size_
#define BANDWIDTH static_cast<JobSpawnerApp*>(application())->bandwidth_

#define STENCIL_SIZE (2*BANDWIDTH)+1
#define PART_SIZE (CHUNK_PER_PART)*CHUNK_SIZE
#define CHUNK_NUM PART_NUM*CHUNK_PER_PART


#define STERILE_FLAG true
#define WITH_DATA true


Main::Main(Application* app) {
  set_application(app);
};

Job * Main::Clone() {
  dbg(DBG_APP, "Cloning main job!\n");
  return new Main(application());
};

void Main::Execute(Parameter params, const DataArray& da) {
  dbg(DBG_APP, "Executing the main job\n");

  assert(CHUNK_SIZE > (2 * BANDWIDTH));
  assert(CHUNK_NUM >= PART_NUM);
  assert(CHUNK_NUM % PART_NUM == 0);

  assert(CHUNK_SIZE > (2 * BANDWIDTH));
  assert(CHUNK_NUM >= PART_NUM);
  assert(CHUNK_NUM % PART_NUM == 0);

  std::vector<job_id_t> job_ids;
  std::vector<logical_data_id_t> d;
  IDSet<logical_data_id_t> read, write;
  IDSet<job_id_t> before, after;
  IDSet<partition_id_t> neighbor_partitions;
  Parameter par;

  GetNewJobID(&job_ids, CHUNK_NUM * 3 + 1);

  if (WITH_DATA) {
    /*
     * Defining partition and data.
     */
    GetNewLogicalDataID(&d, CHUNK_NUM * 3);
    for (size_t i = 0; i < CHUNK_NUM; ++i) {
      GeometricRegion r_l(i * CHUNK_SIZE, 0, 0,
          BANDWIDTH, 1, 1);
      ID<partition_id_t> p_l(i * 3);
      DefinePartition(p_l, r_l);
      DefineData(DATA_NAME, d[i * 3], p_l.elem(), neighbor_partitions);

      GeometricRegion r_m(i * CHUNK_SIZE + BANDWIDTH, 0, 0,
          CHUNK_SIZE - 2 * BANDWIDTH, 1, 1);
      ID<partition_id_t> p_m(i * 3 + 1);
      DefinePartition(p_m, r_m);
      DefineData(DATA_NAME, d[i * 3 + 1], p_m.elem(), neighbor_partitions);

      GeometricRegion r_r(i * CHUNK_SIZE + CHUNK_SIZE - BANDWIDTH, 0, 0,
          BANDWIDTH, 1, 1);
      ID<partition_id_t> p_r(i * 3 + 2);
      DefinePartition(p_r, r_r);
      DefineData(DATA_NAME, d[i * 3 + 2], p_r.elem(), neighbor_partitions);
    }

    /*
     * Spawning init jobs
     */
    for (size_t i = 0; i < CHUNK_NUM; ++i) {
      read.clear(); read.insert(d[i * 3]);
      write.clear(); write.insert(d[i * 3]);
      before.clear();
      SerializeParameter(&par, 0);
      SpawnComputeJob(INIT_JOB_NAME, job_ids[i * 3],
          read, write, before, after, par, STERILE_FLAG);

      read.clear(); read.insert(d[i * 3 + 1]);
      write.clear(); write.insert(d[i * 3 + 1]);
      before.clear();
      SerializeParameter(&par, BANDWIDTH);
      SpawnComputeJob(INIT_JOB_NAME, job_ids[i * 3 + 1],
          read, write, before, after, par, STERILE_FLAG);

      read.clear(); read.insert(d[i * 3 + 2]);
      write.clear(); write.insert(d[i * 3 + 2]);
      before.clear();
      SerializeParameter(&par, CHUNK_SIZE - BANDWIDTH);
      SpawnComputeJob(INIT_JOB_NAME, job_ids[i * 3 + 2],
          read, write, before, after, par, STERILE_FLAG);
    }
  }

  /*
   * Spawning loop job
   */
  read.clear();
  write.clear();
  before.clear();
  if (WITH_DATA) {
    for (size_t j = 0; j < CHUNK_NUM * 3; ++j) {
      before.insert(job_ids[j]);
    }
  }
  after.clear();
  SerializeParameter(&par, LOOP_COUNTER);
  SpawnComputeJob(LOOP_JOB_NAME, job_ids[3 * CHUNK_NUM], read, write, before, after, par);
};

ForLoop::ForLoop(Application* app) {
  set_application(app);
};

Job * ForLoop::Clone() {
  dbg(DBG_APP, "Cloning forLoop job!\n");
  return new ForLoop(application());
};

void ForLoop::Execute(Parameter params, const DataArray& da) {
  dbg(DBG_APP, "Executing the forLoop job\n");

  IDSet<logical_data_id_t> read, write;
  IDSet<job_id_t> before, after;
  Parameter par;

  size_t loop_counter;
  LoadParameter(&params, &loop_counter);

  if (loop_counter > LOOP_CONDITION) {
    /*
     * Spawn the batch of jobs in each stage
     */
    std::vector<job_id_t> stage_job_ids;
    GetNewJobID(&stage_job_ids, STAGE_NUM * PART_NUM);
    std::vector<job_id_t> connector_job_ids;
    GetNewJobID(&connector_job_ids, STAGE_NUM - 1);
    for (size_t s = 0; s < STAGE_NUM; ++s) {
      for (size_t i = 0; i < PART_NUM; ++i) {
        read.clear();
        if (WITH_DATA) {
          GeometricRegion r_r(i * PART_SIZE - BANDWIDTH, 0, 0,
              PART_SIZE + 2 * BANDWIDTH, 1, 1);
          LoadLogicalIdsInSet(this, &read, r_r, DATA_NAME, NULL);
        }
        write.clear();
        if (WITH_DATA) {
          GeometricRegion r_w(i * PART_SIZE, 0, 0,
              PART_SIZE, 1, 1);
          LoadLogicalIdsInSet(this, &write, r_w, DATA_NAME, NULL);
        }
        before.clear();
        if (s > 0) {
          before.insert(connector_job_ids[s - 1]);
        }
        after.clear();
        SpawnComputeJob(STAGE_JOB_NAME, stage_job_ids[s * PART_NUM + i],
            read, write, before, after, par, STERILE_FLAG);
      }
      if (s < (STAGE_NUM - 1)) {
        read.clear();
        write.clear();
        before.clear();
        for (size_t j = 0; j < PART_NUM; ++j) {
          before.insert(stage_job_ids[s * PART_NUM + j]);
        }
        after.clear();
        SpawnComputeJob(CONNECTOR_JOB_NAME, connector_job_ids[s],
            read, write, before, after, par, STERILE_FLAG);
      }
    }

    std::vector<job_id_t> print_job_ids;
    GetNewJobID(&print_job_ids, PART_NUM);

    if (WITH_DATA) {
      /*
       * Spawning the print jobs at the end of each loop
       */
      for (size_t i = 0; i < PART_NUM; ++i) {
        read.clear();
        GeometricRegion r(i * PART_SIZE, 0, 0, PART_SIZE, 1, 1);
        LoadLogicalIdsInSet(this, &read, r, DATA_NAME, NULL);
        write.clear();
        before.clear();
        before.insert(stage_job_ids[(STAGE_NUM - 1) * PART_NUM + i]);
        after.clear();
        SpawnComputeJob(PRINT_JOB_NAME, print_job_ids[i],
            read, write, before, after, par, STERILE_FLAG);
      }
    }

    /*
     * Spawning the next for loop job
     */
    std::vector<job_id_t> forloop_job_id;
    GetNewJobID(&forloop_job_id, 1);
    read.clear();
    write.clear();
    before.clear();
    if (WITH_DATA) {
      for (size_t j = 0; j < PART_NUM; ++j) {
        before.insert(print_job_ids[j]);
      }
    }
    after.clear();
    SerializeParameter(&par, loop_counter - 1);
    SpawnComputeJob(LOOP_JOB_NAME, forloop_job_id[0], read, write, before, after, par);
  } else {
    if (WITH_DATA) {
      std::vector<job_id_t> print_job_id;
      GetNewJobID(&print_job_id, 1);

      /*
       * Spawning final print job
       */
      read.clear();
      GeometricRegion r(0, 0, 0, CHUNK_SIZE * CHUNK_NUM, 1, 1);
      LoadLogicalIdsInSet(this, &read, r, DATA_NAME, NULL);
      write.clear();
      before.clear();
      after.clear();
      SpawnComputeJob(PRINT_JOB_NAME, print_job_id[0],
          read, write, before, after, par, STERILE_FLAG);
    }

    TerminateApplication();
  }
};

Init::Init() {
};

Job * Init::Clone() {
  dbg(DBG_APP, "Cloning init job!\n");
  return new Init();
};

void Init::Execute(Parameter params, const DataArray& da) {
/*
  // std::cout << "Executing the init job\n";
  uint32_t base_val;
  base_val = *(params.idset().begin());
  Vec *d = reinterpret_cast<Vec*>(da[0]);
  for (int i = 0; i < d->size() ; i++)
    d->arr()[i] = base_val + i;
*/
  dbg(DBG_APP, "Executing the init job\n");
  std::vector<int> read_data;
  std::vector<int> write_data;
  LoadDataFromNimbus(this, da, &read_data);

  size_t base_val;
  LoadParameter(&params, &base_val);
  for (size_t i = 0; i < read_data.size() ; ++i) {
    write_data.push_back(base_val + i);
  }

  SaveDataToNimbus(this, da, &write_data);
};


Print::Print() {
};

Job * Print::Clone() {
  dbg(DBG_APP, "Cloning print job!\n");
  return new Print();
};

void Print::Execute(Parameter params, const DataArray& da) {
/*
  // std::cout << "Executing the print job\n";
  // std::cout << "OUTPUT: ";
  for (size_t i = 0; i < da.size(); ++i) {
    Vec *d = reinterpret_cast<Vec*>(da[i]);
    for (int j = 0; j < d->size(); ++j)
      // std::cout << d->arr()[j] << ", ";
  }
  // std::cout << std::endl;
*/
  dbg(DBG_APP, "Executing the print job\n");
  std::vector<int> read_data;
  std::vector<int> write_data;
  LoadDataFromNimbus(this, da, &read_data);

  dbg(DBG_APP, "OUTPUT: ");
  for (size_t i = 0; i < read_data.size(); ++i) {
    dbg(DBG_APP, "%d, ", read_data[i]);
  }
  dbg(DBG_APP, "\n");

  SaveDataToNimbus(this, da, &write_data);
};

Stage::Stage(Application* app) {
  set_application(app);
};

Job * Stage::Clone() {
  dbg(DBG_APP, "Cloning stage job!\n");
  return new Stage(application());
};

void Stage::Execute(Parameter params, const DataArray& da) {
  dbg(DBG_APP, "Executing the stage job\n");

  usleep(JOB_LENGTH_USEC);
};

Connector::Connector(Application* app) {
  set_application(app);
};

Job * Connector::Clone() {
  dbg(DBG_APP, "Cloning connector job!\n");
  return new Connector(application());
};

void Connector::Execute(Parameter params, const DataArray& da) {
  dbg(DBG_APP, "Executing the connector job\n");
  // This job is empty and is meant to just form the job graph.
};





