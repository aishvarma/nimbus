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

#include "applications/physbam/water//physbam_utils.h"

#include "applications/physbam/water//static_config_force.h"

namespace application {

StaticConfigForce::StaticConfigForce(
    const GeometricRegion& global_region) {
  global_region_ = global_region;
  physbam_data_structure_ = NULL;
}

StaticConfigForce::~StaticConfigForce() {
  if (physbam_data_structure_) {
    Destroy();
  }
}

StaticConfigVariable* StaticConfigForce::CreateNew(
    const GeometricRegion& local_region) const {
  StaticConfigForce* force =
      new StaticConfigForce(global_region_);
  force->local_region_ = local_region;
  PhysBAM::GRID<TV> mac_grid;
  mac_grid.Initialize(TV_INT(local_region.dx(),
                             local_region.dy(),
                             local_region.dz()),
                      GridToRange(global_region_, local_region),
                      true);
  force->physbam_data_structure_ = new DataType;
  force->physbam_data_structure_->Resize(mac_grid, 1);
  return force;
}

void StaticConfigForce::Destroy() {
  if (physbam_data_structure_) {
    delete physbam_data_structure_;
  }
}

StaticConfigForce::DataType* StaticConfigForce::GetData() const {
  return physbam_data_structure_;
}

}  // namespace application
