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

#include <inttypes.h>
#include <math.h>
#include "./app.h"
#include "./job.h"
#include "./data.h"
#include "./utils.h"
#include "shared/helpers.h"

#define ITERATION_NUM static_cast<LogisticRegression*>(application())->iteration_num()
#define PARTITION_NUM static_cast<LogisticRegression*>(application())->partition_num()
#define PARTITION_SIZE static_cast<LogisticRegression*>(application())->sample_num_per_partition()

Main::Main(Application* app) {
  set_application(app);
};

Job * Main::Clone() {
  dbg(DBG_APP, "Cloning main job!\n");
  return new Main(application());
};

void Main::Execute(Parameter params, const DataArray& da) {
  dbg(DBG_APP, "Executing the main job!\n");
  assert(PARTITION_NUM > 0);

  IDSet<logical_data_id_t> read, write;
  IDSet<job_id_t> before, after;
  IDSet<partition_id_t> neighbor_partitions;
  Parameter par;

  /*
   * Defining partition and data.
   */
  std::vector<logical_data_id_t> weight_data_ids;
  std::vector<logical_data_id_t> sample_data_ids;
  GetNewLogicalDataID(&sample_data_ids, PARTITION_NUM);
  GetNewLogicalDataID(&weight_data_ids, PARTITION_NUM);

  for (size_t i = 0; i < PARTITION_NUM; ++i) {
    GeometricRegion r(i * PARTITION_SIZE, 0, 0, PARTITION_SIZE, 1, 1);

    ID<partition_id_t> p(i);
    DefinePartition(p, r);

    DefineData(WEIGHT_DATA_NAME, weight_data_ids[i], p.elem(), neighbor_partitions);
    DefineData(SAMPLE_BATCH_DATA_NAME, sample_data_ids[i], p.elem(), neighbor_partitions);
  }

  /*
   * Spawning the init jobs and loop job
   */

  // Spawn the batch of jobs for init stage
  std::vector<job_id_t> init_job_ids;
  GetNewJobID(&init_job_ids, PARTITION_NUM);
  for (size_t i = 0; i < PARTITION_NUM; ++i) {
    GeometricRegion r(i * PARTITION_SIZE, 0, 0, PARTITION_SIZE, 1, 1);
    read.clear();
    write.clear();
    LoadLdoIdsInSet(&write, r, SAMPLE_BATCH_DATA_NAME, NULL);
    before.clear();
    StageJobAndLoadBeforeSet(&before, GRADIENT_JOB_NAME, init_job_ids[i], read, write);
    SerializeParameter(&par, i);
    SpawnComputeJob(INIT_JOB_NAME, init_job_ids[i], read, write, before, after, par, true, r); // NOLINT
  }

  MarkEndOfStage();

  // Spawning loop job
  std::vector<job_id_t> loop_job_id;
  GetNewJobID(&loop_job_id, 1);
  {
    read.clear();
    write.clear();
    before.clear();
    StageJobAndLoadBeforeSet(&before, LOOP_JOB_NAME, loop_job_id[0], read, write, true);
    SerializeParameter(&par, ITERATION_NUM);
    SpawnComputeJob(LOOP_JOB_NAME, loop_job_id[0], read, write, before, after, par);
  }
};

// used for for initializing the samples.
static uint32_t prf(uint32_t x) {
  x += 4698U;
  x = ((x >> 16) ^ x) * 0x45d9f3bU;
  x = ((x >> 16) ^ x) * 0x45d9f3bU;
  x = ((x >> 16) ^ x);
  return x;
}

Init::Init(Application* app) {
  set_application(app);
};

Job * Init::Clone() {
  dbg(DBG_APP, "Cloning init job!\n");
  return new Init(application());
};

void Init::Execute(Parameter params, const DataArray& da) {
  dbg(DBG_APP, "Executing the init job: %lu\n", id().elem());

  size_t base_val;
  LoadParameter(&params, &base_val);

  assert(da.size() == 1);
  assert(da[0]->name() == SAMPLE_BATCH_DATA_NAME);
  SampleBatch *sb = static_cast<SampleBatch*>(da[0]);
  assert(sb->sample_num() == PARTITION_SIZE);

  // omidm
  // double label = (base_val % 2);        // for +1 and 0 labels
  double label = static_cast<double>((base_val % 2) *2) - 1;  // for +1 and -1 labels
  size_t dimension = sb->dimension();
  size_t needed_value = sb->sample_num() * dimension;
  size_t generated_value = 0;
  std::vector<Sample>::iterator iter = sb->samples()->begin();
  for (; iter != sb->samples()->end(); ++iter) {
    iter->set_label(label);
    for (size_t i = 0; i < dimension; i++) {
      // omidm
      // iter->vector()->operator[](i) = label;
      iter->vector()->operator[](i) = prf(base_val * needed_value + generated_value);
      generated_value++;
    }
  }
};


ForLoop::ForLoop(Application* app) {
  set_application(app);
};

Job * ForLoop::Clone() {
  dbg(DBG_APP, "Cloning forLoop job!\n");
  return new ForLoop(application());
};

void ForLoop::Execute(Parameter params, const DataArray& da) {
  dbg(DBG_APP, "Executing the forLoop job: %lu\n", id().elem());

  IDSet<logical_data_id_t> read, write;
  IDSet<job_id_t> before, after;
  Parameter par;

  size_t loop_counter;
  LoadParameter(&params, &loop_counter);

  if (loop_counter > 0) {
    StartTemplate("__MARK_STAT_for_loop");

    // Spawn the batch of jobs for gradient stage
    std::vector<job_id_t> gradient_job_ids;
    GetNewJobID(&gradient_job_ids, PARTITION_NUM);
    for (size_t i = 0; i < PARTITION_NUM; ++i) {
      GeometricRegion r(i * PARTITION_SIZE, 0, 0, PARTITION_SIZE, 1, 1);
      read.clear();
      LoadLdoIdsInSet(&read, r, SAMPLE_BATCH_DATA_NAME, WEIGHT_DATA_NAME, NULL);
      write.clear();
      LoadLdoIdsInSet(&write, r, WEIGHT_DATA_NAME, NULL);
      before.clear();
      StageJobAndLoadBeforeSet(&before, GRADIENT_JOB_NAME, gradient_job_ids[i], read, write);
      SpawnComputeJob(GRADIENT_JOB_NAME, gradient_job_ids[i], read, write, before, after, par, true, r); // NOLINT
    }

    MarkEndOfStage();

    // Spawning the reduction job
    std::vector<job_id_t> reduction_job_id;
    GetNewJobID(&reduction_job_id, 1);
    {
      GeometricRegion r(0, 0, 0, PARTITION_NUM * PARTITION_SIZE, 1, 1);
      read.clear();
      LoadLdoIdsInSet(&read, r, WEIGHT_DATA_NAME, NULL);
      write.clear();
      LoadLdoIdsInSet(&write, r, WEIGHT_DATA_NAME, NULL);
      before.clear();
      StageJobAndLoadBeforeSet(&before, REDUCE_JOB_NAME, reduction_job_id[0], read, write);
      after.clear();
      SpawnComputeJob(REDUCE_JOB_NAME, reduction_job_id[0], read, write, before, after, par, true, r); // NOLINT
    }

    MarkEndOfStage();

    // Spawning the next for loop job
    std::vector<job_id_t> forloop_job_id;
    GetNewJobID(&forloop_job_id, 1);
    {
      read.clear();
      write.clear();
      before.clear();
      StageJobAndLoadBeforeSet(&before, LOOP_JOB_NAME, forloop_job_id[0], read, write, true);
      after.clear();
      SerializeParameter(&par, loop_counter - 1);
      SpawnComputeJob(LOOP_JOB_NAME, forloop_job_id[0], read, write, before, after, par);
    }
    EndTemplate("__MARK_STAT_for_loop");
  } else {
    // StartTemplate("for_loop_end");

    // EndTemplate("for_loop_end");

    TerminateApplication();
  }
};

Gradient::Gradient(Application* app) {
  set_application(app);
};

Job * Gradient::Clone() {
  dbg(DBG_APP, "Cloning gradient job!\n");
  return new Gradient(application());
};

void Gradient::Execute(Parameter params, const DataArray& da) {
  dbg(DBG_APP, "Executing the gradient job: %lu\n", id().elem());
  assert(da.size() == 3);
  Weight *w = NULL;
  SampleBatch *sb = NULL;
  if (da[0]->name() == SAMPLE_BATCH_DATA_NAME) {
    assert(da[1]->name() == WEIGHT_DATA_NAME);
    w = static_cast<Weight*>(da[1]);
    sb = static_cast<SampleBatch*>(da[0]);
  } else {
    assert(da[0]->name() == WEIGHT_DATA_NAME);
    assert(da[1]->name() == SAMPLE_BATCH_DATA_NAME);
    w = static_cast<Weight*>(da[0]);
    sb = static_cast<SampleBatch*>(da[1]);
  }
  assert(da[2]->name() == WEIGHT_DATA_NAME);

  std::vector<double> gradient(w->dimension(), 0);
  std::vector<Sample>::iterator iter = sb->samples()->begin();
  for (; iter != sb->samples()->end(); ++iter) {
    double l = iter->label();
    std::vector<double>* x = iter->vector();
    // omidm
    // double alpha = 0.01;
    // double scale = (l - 1 / (1 + exp(-1 * VectorDotProduct(x, w->vector())))) * alpha;
    double scale = (1 / (1 + exp(l * VectorDotProduct(x, w->vector()))) - 1) * l;
    VectorAddWithScale(&gradient, x, scale);
  }

  w->set_gradient(gradient);
};

Reduce::Reduce(Application *app) {
  set_application(app);
};

Job * Reduce::Clone() {
  dbg(DBG_APP, "Cloning reduce job!\n");
  return new Reduce(application());
};

void Reduce::Execute(Parameter params, const DataArray& da) {
  dbg(DBG_APP, "Executing the reduce job: %lu\n", id().elem());
  assert(da.size() == 2 * PARTITION_NUM);
  Weight *weight = static_cast<Weight*>(da[0]);
  assert(da[0]->name() == WEIGHT_DATA_NAME);
  std::vector<double> reduced = *(weight->vector());

  DataArray::const_iterator iter = da.begin();
  for (size_t i = 0; i < PARTITION_NUM; ++i, ++iter) {
    Weight *w = static_cast<Weight*>(*iter);
    assert((*iter)->name() == WEIGHT_DATA_NAME);
    VectorAddWithScale(&reduced, w->gradient(), 1);
  }
  for (size_t i = 0; i < PARTITION_NUM; ++i, ++iter) {
    Weight *w = static_cast<Weight*>(*iter);
    assert((*iter)->name() == WEIGHT_DATA_NAME);
    w->set_vector(reduced);
  }
  assert(iter == da.end());

  std::cout << "*********** Weight::";
  std::vector<double>::iterator it = reduced.begin();
  for (; it != reduced.end(); ++it) {
    std::cout << ", " <<  *it;
  }
  std::cout << std::endl;
};






