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
 * Definitions and typedef useful for application, data and jobs.
 *
 * Author: Chinmayee Shah <chinmayee.shah@stanford.edu>
 */

#ifndef NIMBUS_APPLICATION_WATER_MULTIPLE_APP_UTILS_H_
#define NIMBUS_APPLICATION_WATER_MULTIPLE_APP_UTILS_H_

#include <time.h>
#include <list>
#include <stdarg.h>

#include "applications/physbam/water//data_names.h"
#include "applications/physbam/water//parameters.h"
#include "src/data/scratch_data_helper.h"
#include "src/shared/geometric_region.h"
#include "src/shared/nimbus.h"
#include "src/shared/nimbus_types.h"
#include "src/shared/timer.h"
#include "src/worker/physical_data_instance.h"

using nimbus::Data;
using nimbus::GeometricRegion;
using nimbus::IDSet;
using nimbus::SerializedData;

namespace application {
    // forward declaration
    struct InitConfig;

    // undefined values
    const int kPNAInt = -1000;
    const int kPNAFloat = -1;
    const nimbus::GeometricRegion kPNAReg(-1000, -1000, -1000, 0, 0, 0);

    typedef std::vector<Data*> DataVec;
    typedef std::vector<DataVec*> DataSetVec;

    // scratch data helpers
    const nimbus::ScratchDataHelper kScratchPosParticles(kGhostW, APP_POS_PARTICLES);
    const nimbus::ScratchDataHelper kScratchNegParticles(kGhostW, APP_NEG_PARTICLES);
    const nimbus::ScratchDataHelper kScratchPosRemParticles(kGhostW, APP_POS_REM_PARTICLES);
    const nimbus::ScratchDataHelper kScratchNegRemParticles(kGhostW, APP_NEG_REM_PARTICLES);

    enum AccessType {READ_ACCESS, WRITE_ACCESS};

    // Note: some hacks that need to be cleaned soon after a meeting/
    // discussion -- one option is to make region a part of data, and
    // let nimbus take care of initializing region correctly when creating
    // the data object
    bool GetTranslatorData(const nimbus::Job *job,
                           const std::string &name,
                           const nimbus::DataArray& da,
                           nimbus::PdiVector *vec,
                           AccessType access_type);
    void GetReadData(const nimbus::Job &job,
                     const nimbus::DataArray &da,
                     nimbus::DataArray *read,
                     bool clear = true);
    void GetWriteData(const nimbus::Job &job,
                      const nimbus::DataArray &da,
                      nimbus::DataArray *read,
                      bool clear = true);
    void GetReadData(const nimbus::Job &job,
                     const std::string &name,
                     const nimbus::DataArray &da,
                     nimbus::DataArray *read,
                     bool clear = true);
    void GetWriteData(const nimbus::Job &job,
                      const std::string &name,
                      const nimbus::DataArray &da,
                      nimbus::DataArray *read,
                      bool clear = true);
    bool GroupSyncData(const nimbus::Job *job,
                       const nimbus::DataArray &da,
                       DataVec *main_copy,
                       DataSetVec *scratch_copies);
    void DestroyTranslatorObjects(nimbus::PdiVector *vec);
    bool GetDataSet(const std::string &name,
                    const nimbus::DataArray &da,
                    std::set<Data * > *ds);
    nimbus::Data* GetFirstData(const std::string &name,
                               const nimbus::DataArray &da);
    nimbus::Data* GetTheOnlyData(const nimbus::Job *job,
                                 const std::string &name,
                                 const nimbus::DataArray& da,
                                 AccessType access_type);

   // Note: lets make read/ write sets if possible, and also have separate
   // read/ write instead of one DataArray passed to a job/ a better indexing
    bool Contains(nimbus::IDSet<nimbus::logical_data_id_t> data_set,
                  nimbus::logical_data_id_t  id);

    void LoadLogicalIdsInSet(nimbus::Job* job,
        nimbus::IDSet<nimbus::logical_data_id_t>* set,
        const nimbus::GeometricRegion& region, ...);

    bool SerializeParameter(
        const int frame,
        const T time,
        const T dt,
        const int rank,
        const GeometricRegion &global_region,
        const GeometricRegion &local_region,
        const int iteration,
        std::string *result);
    bool LoadParameter(
        const std::string str,
        InitConfig *config);

    class ScopeTimer {
     public:
      ScopeTimer(const std::string& name);
      ~ScopeTimer();
      static void Initialize(bool activated);
     private:
      static FILE* log_file_;
      static bool activated_;
      std::string name_;
      struct timespec start_time_;
    };
} // namespace application

#endif  // NIMBUS_APPLICATION_WATER_MULTIPLE_APP_UTILS_H_
