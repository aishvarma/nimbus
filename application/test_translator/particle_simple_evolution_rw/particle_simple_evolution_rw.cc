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
  * Tests simple read and write operations.
  * Author: Hang Qu <quhang@stanford.edu>
  */

#include <vector>

#include "data/physbam/physbam_data.h"
#include "data/physbam/physbam_include.h"
#include "../translator_physbam_test.h"  // NOLINT

const int_dimension_t X = 1;
const int_dimension_t Y = 1;
const int_dimension_t Z = 1;
const int_dimension_t DX = 1;
const int_dimension_t DY = 1;
const int_dimension_t DZ = 1;
const int_dimension_t SIZE = DX * DY * DZ;
const int AVG_PARTICLES = 10;
const int TOTAL_PARTICLES = SIZE * AVG_PARTICLES * 5;
const int NUM_GHOST_CELLS = 2;

float getX() {
  double val = drand48();
  val *= static_cast<double>(DX);
  val += static_cast<double>(X);
  return static_cast<float>(val);
}

float getY() {
  double val = drand48();
  val *= static_cast<double>(DY);
  val += static_cast<double>(Y);
  return static_cast<float>(val);
}

float getZ() {
  double val = drand48();
  val *= static_cast<double>(DZ);
  val += static_cast<double>(Z);
  return static_cast<float>(val);
}

// The goto clause prevents me from declaring variables. So switch.
void Error() {
  printf("Tests failed. Use dbg=error to observe errors.\n");
  exit(1);
  return;
}

typedef TranslatorPhysBAMTest<PhysBAM::VECTOR<float, 3> > Translator;
typedef PhysBAM::LEVELSET_CALLBACKS<PhysBAM::GRID<Translator::TV> > Callbacks;
typedef typename PhysBAM::PARTICLE_LEVELSET_EVOLUTION_UNIFORM<Translator::Grid> ParticleLevelsetEvolution;
typedef typename PhysBAM::GEOMETRY_BOUNDARY_POLICY<Translator::Grid>::BOUNDARY_PHI_WATER BoundaryPhiWater;

int main(int argc, char *argv[]) {
  dbg_init();
  PhysBAM::LOG::Initialize_Logging();
  PhysBAM::Initialize_Particles();
  PhysBAM::Initialize_Read_Write_General_Structures();
  Callbacks* callbacks = new Callbacks();

  // Sets up test data.
  float* floats = new float[TOTAL_PARTICLES];
  float* floatSource = new float[TOTAL_PARTICLES];
  for (int i = 0; i < TOTAL_PARTICLES; i+=5) {
    floatSource[i + 0] = floats[i + 0] = getX();
    floatSource[i + 1] = floats[i + 1] = getY();
    floatSource[i + 2] = floats[i + 2] = getZ();
    floatSource[i + 3] = floats[i + 3] = 1.0;
    floatSource[i + 4] = floats[i + 4] = 1.0;
  }

  Translator translator;

  int_dimension_t dimensions[] = {X, Y, Z, DX, DY, DZ};
  nimbus::GeometricRegion* r = new nimbus::GeometricRegion(dimensions);
  nimbus::LogicalDataObject* ldo = new LogicalDataObject(1, "velocity", r);

  PhysBAMData* pd = new PhysBAMData();
  pd->set_buffer((char*)floats, TOTAL_PARTICLES * sizeof(float));  // NOLINT
  PhysicalDataInstance* instance = new PhysicalDataInstance(1, ldo, pd, 0);
  CPdiVector vec;
  vec.push_back(instance);

  // TODO(quhang): Need more time to figure out whether the range specification
  // corresponds to original PhysBAM.
  PhysBAM::RANGE<Translator::TV> range_input(
      X, X+DX, Y, Y+DY, Z, Z+DZ);
  Translator::Grid grid(Translator::TV_INT(DX, DY, DZ), range_input, true);

  ParticleLevelsetEvolution evolution(grid, NUM_GHOST_CELLS);

  evolution.Initialize_Domain(grid);
  evolution.particle_levelset.Set_Band_Width(6);
  evolution.Set_Time(0);
  evolution.Set_CFL_Number(0.9);
  evolution.Set_Number_Particles_Per_Cell(2 * AVG_PARTICLES);
  evolution.Use_Particle_Levelset(true);
  evolution.Set_Levelset_Callbacks(*callbacks);

  Translator::ParticleContainer &particle_container =
      evolution.particle_levelset;
  nimbus::GeometricRegion region(dimensions);
  float* result;
  bool pass = true;
  pass = translator.ReadParticles(&region, &vec, particle_container, true);

  if (!pass) {
    Error();
  }

  for (int i = 0; i < TOTAL_PARTICLES; i++) {
    floats[i] = 1.0;
  }

  pass = translator.WriteParticles(&region, &vec, particle_container, true);
  if (!pass) {
    Error();
  }

  // @phil Cannot compare to float array, which is deleted by PhysBAM data.
  // Not sure if this is expected behavior? --quhang
  // TODO(quhang) Don't assume particle values are different.
  result = reinterpret_cast<float*>(
      reinterpret_cast<PhysBAMData*>(instance->data())->buffer());

  if (reinterpret_cast<PhysBAMData*>(instance->data())->size() !=
      TOTAL_PARTICLES * sizeof(float)) {
    printf("The number of particles is incorrect!\n");
    Error();
  }
  std::vector<bool> checked(TOTAL_PARTICLES / 5, false);
  for (int i = 0; i < TOTAL_PARTICLES / 5; ++i) {
    // Check whether the ith particle in floatSource exists in result.
    bool findit = false;
    // Go throught each particle in result.
    for (int j = 0; j < TOTAL_PARTICLES / 5; ++j)
      if (!checked[j]) {
        bool flag = true;
        for (int k = 0; k < 5; ++k)
          if (floatSource[i*5+k] != result[j*5+k])
            flag = false;
        if (flag) {
          findit = true;
          checked[j] = true;
          break;
        }
      }
    if (!findit) {
      printf("The %dth Particle set is not found in the output.\n", i);
      Error();
    }
  }

  printf("Passed all tests successfully.\n");
}