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
 * This file contains job PROJECTION_CALCULATE_BOUNDARY_CONDITION that:
 *     calculates all the boundary condition required by projection.
 * The parameters of PROJECTION_CALCULATE_BOUNDARY_CONDITION:
 *     frame number, simulation time, dt.
 * The read set of PROJECTION_CALCULATE_BOUNDARY_CONDITION:
 *     velocity, pressure, levelset.
 * The write set(not sure) of PROJECTION_CALCULATE_BOUNDARY_CONDITION:
 *     velocity, pressure, psi_D, psi_N, filled_region_colors, divergence.
 *
 * This job should be broken into finer-grained jobs in the future.
 *
 * Author: Hang Qu <quhang@stanford.edu>
 */

#ifndef NIMBUS_APPLICATION_WATER_MULTIPLE_PROJECTION_JOB_PROJECTION_CALCULATE_BOUNDARY_CONDITION_H_
#define NIMBUS_APPLICATION_WATER_MULTIPLE_PROJECTION_JOB_PROJECTION_CALCULATE_BOUNDARY_CONDITION_H_

#include "shared/nimbus.h"

namespace application {

class JobProjectionCalculateBoundaryCondition : public nimbus::Job {
 public:
  explicit JobProjectionCalculateBoundaryCondition(nimbus::Application *app);
  virtual void Execute(
      nimbus::Parameter params,
      const nimbus::DataArray& da);
  virtual nimbus::Job* Clone();
};

} // namespace application

#endif  // NIMBUS_APPLICATION_WATER_MULTIPLE_PROJECTION_JOB_PROJECTION_CALCULATE_BOUNDARY_CONDITION_H_
