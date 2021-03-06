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
  * TranslatorPhysBAM is a class for translating Nimbus physical objects into
  * PhysBAM data objects. It is templated to make it a little easier
  * to handle PhysBAM's generality, taking a simple template parameter
  * of a PhysBAM VECTOR. This VECTOR is typically a 2D or 3D float: it
  * represents a point in space. The class derives the scalar type
  * (typically float) from this VECTOR, as well as the dimensionality.
  *
  * Author: Philip Levis <pal@cs.stanford.edu>
  */

#ifndef NIMBUS_SRC_DATA_PHYSBAM_TRANSLATOR_PHYSBAM_OLD_H_
#define NIMBUS_SRC_DATA_PHYSBAM_TRANSLATOR_PHYSBAM_OLD_H_

#include <sys/syscall.h>
#include <algorithm>
#include <cmath>
#include <list>
#include <string>

#include "src/data/physbam/physbam_include.h"
#include "src/data/physbam/physbam_data.h"

#include "src/shared/log.h"
#include "src/shared/nimbus_types.h"
#include "src/shared/geometric_region.h"
#include "src/worker/physical_data_instance.h"
#include "src/worker/worker.h"

#define TRANSLATE_OLD_LOG_H "[Translator]"

namespace nimbus {

  template <class VECTOR_TYPE> class TranslatorPhysBAMOld {
  public:
    typedef VECTOR_TYPE TV;
    typedef typename TV::template REBIND<int>::TYPE TV_INT;
    typedef PhysBAM::GRID<TV> Grid;
    typedef typename TV::SCALAR scalar_t;
    typedef PhysBAM::VECTOR<int_dimension_t, 3> Dimension3Vector;
    typedef PhysBAM::VECTOR<int, 3> Int3Vector;
    typedef typename PhysBAM::FACE_INDEX<TV::dimension> FaceIndex;
    // typedef typename PhysBAM::ARRAY<scalar_t, FaceIndex> FaceArray;

    // typedef typename PhysBAM::PARTICLE_LEVELSET_PARTICLES<TV> Particles;
    // typedef typename PhysBAM::ARRAY<Particles*, PhysBAM::VECTOR<int, 3> > ParticlesArray;
    //  typedef typename PhysBAM::ARRAY<Particles*, int> ParticlesArray;

    // Container class for particles and removed particles.
    typedef typename PhysBAM::PARTICLE_LEVELSET_UNIFORM<Grid> ParticleContainer;
    // Particle bucket. Each node has a linked list of particle buckets.
    typedef typename PhysBAM::PARTICLE_LEVELSET_PARTICLES<TV> ParticleBucket;
    // Particle array, indexed by node.
    typedef typename PhysBAM::ARRAY<ParticleBucket*, TV_INT> ParticleArray;

    typedef typename PhysBAM::PARTICLE_LEVELSET_REMOVED_PARTICLES<TV>
        RemovedParticleBucket;
    typedef typename PhysBAM::ARRAY<RemovedParticleBucket*, TV_INT>
        RemovedParticleArray;

    typedef typename PhysBAM::PARTICLE_LEVELSET<Grid> ParticleLevelset;
    // typedef typename PhysBAM::ARRAY<scalar_t, Int3Vector> ScalarArray;

    enum {
      X_COORD = 1,
      Y_COORD = 2,
      Z_COORD = 3
    };

    explicit TranslatorPhysBAMOld() {}
    virtual ~TranslatorPhysBAMOld() {}

    // Data structures used to format particles in PhysBAMData.
    // Should be changed to protocol buffer later for compatibility.
    // TODO(quhang) .
    struct ParticleInternal {
      scalar_t position[3];
      scalar_t radius;
      uint16_t quantized_collision_distance;
      int32_t id;
    };

    struct RemovedParticleInternal : public ParticleInternal {
      scalar_t v[3];
    };

    static Log *log;

    /** Take a FaceArray described by region and read its data from the
     *  PhysicalDataInstance objects in the objects array.
     */
    template<typename T> void ReadFaceArray(
        const GeometricRegion* region,
        const int_dimension_t shift[3],
        const PdiVector* objects,
        typename PhysBAM::ARRAY<T, FaceIndex>* fa) {
      if (log) {
          std::stringstream msg;
          pid_t tid = syscall(SYS_gettid);
          msg << "### TID: " << tid << "  Read Face Array (Old Translator) start : " << log->GetTime(); // NOLINT
          log->WriteToFile(msg.str());
      }
      PhysBAM::ARRAY<T, FaceIndex> flag;
      flag = *fa;
      flag.Fill(0);
      if (objects != NULL) {
        PdiVector::const_iterator iter = objects->begin();
        for (; iter != objects->end(); ++iter) {
          const PhysicalDataInstance* obj = *iter;
          Dimension3Vector overlap = GetOverlapSize(obj->region(), region);
          if (HasOverlap(overlap)) {
            std::string reg_str = region->ToNetworkData();
            PhysBAMData* data = static_cast<PhysBAMData*>(obj->data());
            T* buffer = reinterpret_cast<T*>(data->buffer());

            Dimension3Vector src = GetOffset(obj->region(), region);
            Dimension3Vector dest = GetOffset(region, obj->region());

            //  x, y and z values are stored separately due to the
            // difference in number of x, y and z values in face arrays
            for (int dim = X_COORD; dim <= Z_COORD; dim++) {
              int mult_x = 1;
              int mult_y = obj->region()->dx();
              int mult_z = obj->region()->dy() * obj->region()->dx();
              int range_x = overlap(X_COORD);
              int range_y = overlap(Y_COORD);
              int range_z = overlap(Z_COORD);
              int src_offset = 0;
              switch (dim) {
                  case X_COORD:
                    range_x += 1;
                    mult_y  += 1;
                    mult_z  += obj->region()->dy();
                    break;
                  case Y_COORD:
                    range_y += 1;
                    mult_z  += obj->region()->dx();
                    src_offset += (obj->region()->dx() + 1) *
                                  (obj->region()->dy()) *
                                  (obj->region()->dz());
                    break;
                  case Z_COORD:
                    range_z += 1;
                    src_offset += ((obj->region()->dx()) *
                                  (obj->region()->dy()+1) *
                                  (obj->region()->dz())) +
                                  ((obj->region()->dx() + 1) *
                                  (obj->region()->dy()) *
                                  (obj->region()->dz()));
                    break;
                }
              for (int z = 0; z < range_z; z++) {
                for (int y = 0; y < range_y; y++) {
                  for (int x = 0; x < range_x; x++) {
                    int source_x = x + src(X_COORD);
                    int source_y = y + src(Y_COORD);
                    int source_z = z + src(Z_COORD);

                    int source_index = source_x * mult_x +
                                       source_y * mult_y +
                                       source_z * mult_z;
                    source_index += src_offset;

                    int dest_x = x + dest(X_COORD) + region->x() - shift[0];
                    int dest_y = y + dest(Y_COORD) + region->y() - shift[1];
                    int dest_z = z + dest(Z_COORD) + region->z() - shift[2];

                    typename PhysBAM::VECTOR<int, 3>
                      destinationIndex(dest_x, dest_y, dest_z);

                    // The PhysBAM FACE_ARRAY object abstracts away whether
                    // the data is stored in struct of array or array of struct
                    // form (in practice, usually struct of arrays.
                    assert(source_index < data->size() / (int) sizeof(T) // NOLINT
                           && source_index >= 0);
                    if (flag(dim, destinationIndex) == 0) {
                      (*fa)(dim, destinationIndex) = buffer[source_index];
                      flag(dim, destinationIndex) = 1;
                    } else if (flag(dim, destinationIndex) == 1) {
                      if ((*fa)(dim, destinationIndex)
                          != buffer[source_index]) {
                        (*fa)(dim, destinationIndex) += buffer[source_index];
                        (*fa)(dim, destinationIndex) /= 2;
                      }
                      flag(dim, destinationIndex) = 2;
                    } else {
                      // TODO(quhang) needs a more elegant solution.
                      assert(false);
                    }
                  }
                }
              }
            }
          }
        }
      }
      if (log) {
          std::stringstream msg;
          pid_t tid = syscall(SYS_gettid);
          msg << "### TID: " << tid << "  Read Face Array (Old Translator) end : " << log->GetTime(); // NOLINT
          log->WriteToFile(msg.str());
      }
    }

    /* Helper function to specify the element type. Implicit template type
     * inference seems unreliable.
     */
    void ReadFaceArrayFloat(
        const GeometricRegion* region,
        const int_dimension_t shift[3],
        const PdiVector* objects,
        typename PhysBAM::ARRAY<float, FaceIndex>* fa) {
      ReadFaceArray<float>(region, shift, objects, fa);
    }

    void ReadFaceArrayBool(
        const GeometricRegion* region,
        const int_dimension_t shift[3],
        const PdiVector* objects,
        typename PhysBAM::ARRAY<bool, FaceIndex>* fa) {
      ReadFaceArray<bool>(region, shift, objects, fa);
    }

    /** Take a FaceArray described by region and write it out to the
     *  PhysicalDataInstance objects in the objects array. */
    template<typename T> bool WriteFaceArray(
        const GeometricRegion* region,
        const int_dimension_t shift[3],
        PdiVector* objects,
        typename PhysBAM::ARRAY<T, FaceIndex>* fa) {
      if (log) {
          std::stringstream msg;
          pid_t tid = syscall(SYS_gettid);
          msg << "### TID: " << tid << "  Write Face Array (Old Translator) start : " << log->GetTime(); // NOLINT
          log->WriteToFile(msg.str());
      }
      int_dimension_t region_size = 0;
      region_size += (region->dx() + 1) * region->dy() * region->dz();
      region_size += region->dx() * (region->dy() + 1) * region->dz();
      region_size += region->dx() * region->dy() * (region->dz() + 1);
      if (region_size != fa->buffer_size) {
        dbg(DBG_WARN, "WARN: writing a face array of size %i for a region of size %i and the two sizes should be equal. This check is wrong so you can ignore this warning. I need to determine correct check. -pal\n", fa->buffer_size, region_size);  // NOLINT
        //  return false;
      }

      if (objects != NULL) {
        PdiVector::iterator iter = objects->begin();

        // Loop over the Nimbus objects, copying the relevant PhysBAM data
        // into each one
        for (; iter != objects->end(); ++iter) {
          const PhysicalDataInstance* obj = *iter;
          Dimension3Vector overlap = GetOverlapSize(obj->region(), region);
          if (!HasOverlap(overlap)) {continue;}

          PhysBAMData* data = static_cast<PhysBAMData*>(obj->data());
          T* buffer = reinterpret_cast<T*>(data->buffer());

          Dimension3Vector src = GetOffset(region, obj->region());
          Dimension3Vector dest = GetOffset(obj->region(), region);

          //  x, y and z values are stored separately due to the
          // difference in number of x, y and z values in face arrays
          for (int dim = X_COORD; dim <= Z_COORD; dim++) {
            int mult_x = 1;
            int mult_y = obj->region()->dx();
            int mult_z = obj->region()->dy() * obj->region()->dx();
            int range_x = overlap(X_COORD);
            int range_y = overlap(Y_COORD);
            int range_z = overlap(Z_COORD);
            int dst_offset = 0;
            switch (dim) {
                case X_COORD:
                  range_x += 1;
                  mult_y  += 1;
                  mult_z  += obj->region()->dy();
                  break;
                case Y_COORD:
                  range_y += 1;
                  mult_z  += obj->region()->dx();
                  dst_offset += (obj->region()->dx() + 1) *
                                (obj->region()->dy()) *
                                (obj->region()->dz());
                  break;
                case Z_COORD:
                  range_z += 1;
                  dst_offset += ((obj->region()->dx()) *
                                (obj->region()->dy()+1) *
                                (obj->region()->dz())) +
                                ((obj->region()->dx() + 1) *
                                (obj->region()->dy()) *
                                (obj->region()->dz()));
                  break;
              }
            for (int z = 0; z < range_z; z++) {
              for (int y = 0; y < range_y; y++) {
                for (int x = 0; x < range_x; x++) {
                  int dest_x = x + dest(X_COORD);
                  int dest_y = y + dest(Y_COORD);
                  int dest_z = z + dest(Z_COORD);

                  int destination_index = dest_x * mult_x +
                                          dest_y * mult_y +
                                          dest_z * mult_z;
                  destination_index += dst_offset;

                  int source_x = x + src(X_COORD) + region->x() - shift[0];
                  int source_y = y + src(Y_COORD) + region->y() - shift[1];
                  int source_z = z + src(Z_COORD) + region->z() - shift[2];

                  typename PhysBAM::VECTOR<int, 3>
                    sourceIndex(source_x, source_y, source_z);

                  // The PhysBAM FACE_ARRAY object abstracts away whether
                  // the data is stored in struct of array or array of struct
                  // form (in practice, usually struct of arrays
                  assert(destination_index < data->size() / (int) sizeof(T) && destination_index >= 0); // NOLINT
                  buffer[destination_index] = (*fa)(dim, sourceIndex);
                }
              }
            }
          }
        }
      }
      if (log) {
          std::stringstream msg;
          pid_t tid = syscall(SYS_gettid);
          msg << "### TID: " << tid << "  Write Face Array (Old Translator) end : " << log->GetTime(); // NOLINT
          log->WriteToFile(msg.str());
      }
      return true;
    }

    /* Helper function to specify the element type. Implicit template type
     * inference seems unreliable.
     */
    bool WriteFaceArrayFloat(
        const GeometricRegion* region,
        const int_dimension_t shift[3],
        PdiVector* objects,
        typename PhysBAM::ARRAY<float, FaceIndex>* fa) {
      return WriteFaceArray<float>(region, shift, objects, fa);
    }

    bool WriteFaceArrayBool(
        const GeometricRegion* region,
        const int_dimension_t shift[3],
        PdiVector* objects,
        typename PhysBAM::ARRAY<bool, FaceIndex>* fa) {
      return WriteFaceArray<bool>(region, shift, objects, fa);
    }

    /* Reads the particle data from the PhysicalDataInstances "instances",
     * limited by the corresponding global region calculated from shifting the
     * local region "region" by the offset "shift", into the local region
     * "region" of the PhysBAM particle container "particle_container".
     *
     * "positive" option specifies whether to work on positive particles or
     * negative particles.
     * "merge" option specifies whether to keep original particle data in
     * "particle_container".
     *
     * More explanation for the region calculation, if,
     *     region = {-1, -1, -1, 12, 12, 12}, shift = {10, 10, 10},
     * then we will copy the data in "instances" limited by global region:
     *     {9, 9, 9, 22, 22, 22}
     * into "particle_container" for the local region:
     *     {-1, -1, -1, 12, 12, 12}.
     */
    bool ReadParticles(const GeometricRegion* region,
                       const int_dimension_t shift[3],
                       const PdiVector* instances,
                       ParticleContainer& particle_container,
                       const int_dimension_t kScale,
                       bool positive,
                       bool merge = false) {
      if (log) {
          std::stringstream msg;
          pid_t tid = syscall(SYS_gettid);
          msg << "### TID: " << tid << "  Read Particles (Old Translator) start : " << log->GetTime(); // NOLINT
          log->WriteToFile(msg.str());
      }
      if (positive) {
        dbg(DBG_TRANSLATE,
            TRANSLATE_OLD_LOG_H"Start ReadParticles for positive particles\n");
      } else {
        dbg(DBG_TRANSLATE,
            TRANSLATE_OLD_LOG_H"Start ReadParticles for negative particles\n");
      }
      Log timer;
      int64_t counter, counter1, counter2;

      ParticleArray* particles;
      if (positive) {
        particles = &particle_container.positive_particles;
      } else {
        particles = &particle_container.negative_particles;
      }

      timer.StartTimer();
      counter = 0;
      // Checks whether the geometric region in the particle array is valid, and
      // clears corresponding buckets inside the geometric region if necessary.
      for (int z = region->z(); z <= region->z() + region->dz(); z++)
        for (int y = region->y(); y <= region->y() + region->dy(); y++)
          for (int x = region->x(); x <= region->x() + region->dx(); x++) {
            ++counter;
            TV_INT bucket_index(x, y, z);
            if (!particles->Valid_Index(bucket_index)) {
              dbg(DBG_WARN, "Bucket index (%d, %d, %d) out of range.\n",
                            x, y, z);
              // Warning: might be too strict.
              if (log) {
                  std::stringstream msg;
                  pid_t tid = syscall(SYS_gettid);
                  msg << "### TID: " << tid << "  Read Particles (Old Translator) end : " << log->GetTime(); // NOLINT
                  log->WriteToFile(msg.str());
              }
              return false;
            }
            if (!merge) {
              particle_container.Free_Particle_And_Clear_Pointer(
                  (*particles)(bucket_index));
            }
          }
      dbg(DBG_TRANSLATE,
          TRANSLATE_OLD_LOG_H
          "In ReadParticles, clean %ld buckets in %0.2f seconds\n",
          counter, timer.GetTime());

      if (instances == NULL) {
        dbg(DBG_WARN, "Physical data instances are empty.\n");
        if (log) {
            std::stringstream msg;
            pid_t tid = syscall(SYS_gettid);
            msg << "### TID: " << tid << "  Read Particles (Old Translator) end : " << log->GetTime(); // NOLINT
            log->WriteToFile(msg.str());
        }
        return false;
      }

      timer.StartTimer();
      counter1 = 0;
      counter2 = 0;
      PdiVector::const_iterator iter = instances->begin();
      for (; iter != instances->end(); ++iter) {
        const PhysicalDataInstance* instance = *iter;
        PhysBAMData* data = static_cast<PhysBAMData*>(instance->data());
        ParticleInternal* buffer =
            reinterpret_cast<ParticleInternal*>(data->buffer());
        ParticleInternal* buffer_end = buffer + static_cast<int>(data->size())
             / static_cast<int>(sizeof(ParticleInternal));

        for (ParticleInternal* p = buffer; p != buffer_end; ++p) {
          VECTOR_TYPE absolute_position;
          absolute_position.x = p->position[0];
          absolute_position.y = p->position[1];
          absolute_position.z = p->position[2];
          ++counter1;
          // TODO(quhang) Needs to deal with the particles that lies exactly on
          // the boundary.
          if (absolute_position.x >= region->x() + shift[0] &&
              absolute_position.x < region->x() + region->dx() + shift[0] &&
              absolute_position.y >= region->y() + shift[1] &&
              absolute_position.y < region->y() + region->dy() + shift[1] &&
              absolute_position.z >= region->z() + shift[2] &&
              absolute_position.z < region->z() + region->dz() + shift[2]) {
            ++counter2;
            TV_INT bucket_index(round(absolute_position.x - shift[0]),
                                round(absolute_position.y - shift[1]),
                                round(absolute_position.z - shift[2]));
            assert(particles->Valid_Index(bucket_index));
            // NOTE(By Chinmayee): Please comment out these changes and don't
            // delete them when pushing any updates, till we verify that the
            // code works correctly, and does not give any assertion failure or
            // seg fault on both Linux and Mac.
            if (!(*particles)(bucket_index)) {
              (*particles)(bucket_index) = particle_container.
                  Allocate_Particles(particle_container.template_particles);
            }
            ParticleBucket* particle_bucket =
                (*particles)(bucket_index);

            // Note that Add_Particle traverses a linked list of particle
            // buckets, so it's O(N^2) time. Blech.
            int index = particle_container.Add_Particle(particle_bucket);
            particle_bucket->X(index) =
                (absolute_position - 1.0) / (float) kScale; // NOLINT
            particle_bucket->radius(index) = p->radius;
            particle_bucket->quantized_collision_distance(index) =
              p->quantized_collision_distance;
            if (particle_container.store_unique_particle_id) {
              PhysBAM::ARRAY_VIEW<int>* id = particle_bucket->array_collection->
                  template Get_Array<int>(PhysBAM::ATTRIBUTE_ID_ID);
              (*id)(index) = p->id;
            }
          }
        }  // End the loop for buffer.
      }
      dbg(DBG_TRANSLATE,
          TRANSLATE_OLD_LOG_H
          "In ReadParticles, go through %ld particles and read %ld particles"
          " in %0.2f seconds\n",
          counter1, counter2, timer.GetTime());
      if (log) {
          std::stringstream msg;
          pid_t tid = syscall(SYS_gettid);
          msg << "### TID: " << tid << "  Read Particles (Old Translator) end : " << log->GetTime(); // NOLINT
          log->WriteToFile(msg.str());
      }
      return true;
    }


    /* Writes the particle data from PhysBAM particle container
     * "particle_container", limited the local region "region", into the
     * corresponding global region of physical instances "instances" after
     * performing the coordinate shifting specified by "shift".
     *
     * "positive" option specifies whether to work on positive particles or
     * negative particles.
     *
     * More explanation for the region calculation, if,
     *     region = {-1, -1, -1, 12, 12, 12}, shift = {10, 10, 10},
     * then we will copy the data of "particle_container" in the local region:
     *     {-1, -1, -1, 12, 12, 12}
     * into the global region of corresponding "instances":
     *     {9, 9, 9, 22, 22, 22}.
     */
    bool WriteParticles(const GeometricRegion* region,
                        const int_dimension_t shift[3],
                        PdiVector* instances,
                        ParticleContainer& particle_container,
                        const int_dimension_t kScale,
                        bool positive) {
      if (log) {
          std::stringstream msg;
          pid_t tid = syscall(SYS_gettid);
          msg << "### TID: " << tid << "  Write Particles (Old Translator) start : " << log->GetTime(); // NOLINT
          log->WriteToFile(msg.str());
      }
      if (positive) {
        dbg(DBG_TRANSLATE,
            TRANSLATE_OLD_LOG_H"Start WriteParticles for positive particles\n");
      } else {
        dbg(DBG_TRANSLATE,
            TRANSLATE_OLD_LOG_H"Start WriteParticles for negative particles\n");
      }
      Log timer;
      int64_t counter1, counter2;

      PdiVector::iterator iter = instances->begin();
      for (; iter != instances->end(); ++iter) {
        const PhysicalDataInstance* instance = *iter;
        PhysBAMData* data = static_cast<PhysBAMData*>(instance->data());
        data->ClearTempBuffer();
      }

      ParticleArray* particles;
      if (positive) {
        particles = &particle_container.positive_particles;
      } else {
        particles = &particle_container.negative_particles;
      }

      timer.StartTimer();
      counter1 = 0;
      counter2 = 0;
      // Loop through each particle bucket in the specified region.
      for (int z = region->z(); z <= region->z() + region->dz(); z++)
        for (int y = region->y(); y <= region->y() + region->dy(); y++)
          for (int x = region->x(); x <= region->x() + region->dx(); x++) {
            TV_INT bucket_index(x, y, z);
            if (!particles->Valid_Index(bucket_index)) {
              dbg(DBG_WARN, "Bucket index (%d, %d, %d) out of range.\n",
                            x, y, z);
              if (log) {
                  std::stringstream msg;
                  pid_t tid = syscall(SYS_gettid);
                  msg << "### TID: " << tid << "  Write Particles (Old Translator) end : " << log->GetTime(); // NOLINT
                  log->WriteToFile(msg.str());
              }
              return false;
            }
            PdiVector::iterator iter = instances->begin();
            for (; iter != instances->end(); ++iter) {
              // Iterate across instances, checking each one.
              const PhysicalDataInstance* instance = *iter;
              GeometricRegion* instanceRegion = instance->region();
              // TODO(quhang) needs to double check the margin setting.
              const int_dimension_t kMargin = 1;
              if (x + shift[0] <
                  instanceRegion->x() - kMargin ||
                  x + shift[0] >
                  instanceRegion->x() + instanceRegion->dx() + kMargin ||
                  y + shift[1] <
                  instanceRegion->y() - kMargin ||
                  y + shift[1] >
                  instanceRegion->y() + instanceRegion->dy() + kMargin ||
                  z + shift[2] <
                  instanceRegion->z() - kMargin ||
                  z + shift[2] >
                  instanceRegion->z() + instanceRegion->dz() + kMargin) {
                continue;
              }
              ParticleBucket* particle_bucket = (*particles)(bucket_index);
              while (particle_bucket) {
                for (int i = 1; i <= particle_bucket->array_collection->Size();
                     i++) {
                  VECTOR_TYPE particle_position = particle_bucket->X(i);
                  VECTOR_TYPE absolute_position =
                      particle_position * (float) kScale + 1.0; // NOLINT
                  ++counter1;
                  // TODO(quhang) Needs to deal with the case when the particle
                  // lies exactly on the boundary.
                  // If it's inside the region of the physical data instance.
                  if (absolute_position.x >=
                      instanceRegion->x() &&
                      absolute_position.x <
                      (instanceRegion->x() + instanceRegion->dx()) &&
                      absolute_position.y >=
                      instanceRegion->y() &&
                      absolute_position.y <
                      (instanceRegion->y() + instanceRegion->dy()) &&
                      absolute_position.z >=
                      instanceRegion->z() &&
                      absolute_position.z <
                      (instanceRegion->z() + instanceRegion->dz())) {
                    ++counter2;
                    ParticleInternal particle_buffer;
                    particle_buffer.position[0] = absolute_position.x;
                    particle_buffer.position[1] = absolute_position.y;
                    particle_buffer.position[2] = absolute_position.z;
                    particle_buffer.radius = particle_bucket->radius(i);
                    particle_buffer.quantized_collision_distance =
                        particle_bucket->quantized_collision_distance(i);
                    if (particle_container.store_unique_particle_id) {
                      PhysBAM::ARRAY_VIEW<int>* id =
                          particle_bucket->array_collection->
                          template Get_Array<int>(PhysBAM::ATTRIBUTE_ID_ID);
particle_buffer.id = (*id)(i);
                    }
                    PhysBAMData* data =
                        static_cast<PhysBAMData*>(instance->data());
                    data->AddToTempBuffer(
                        reinterpret_cast<char*>(&particle_buffer),
                        sizeof(particle_buffer));
                  }
                }
                particle_bucket = particle_bucket->next;
              }  // Finish looping through all particles.
            }
          }

      dbg(DBG_TRANSLATE,
          TRANSLATE_OLD_LOG_H
          "In WriteParticles, scan %ld particles and write %ld particles"
          " in %0.2f seconds\n",
          counter1, counter2, timer.GetTime());

      // Now that we've copied particles into the temporary data buffers,
      // commit the results.
      iter = instances->begin();
      for (; iter != instances->end(); ++iter) {
        const PhysicalDataInstance* instance = *iter;
        PhysBAMData* data = static_cast<PhysBAMData*>(instance->data());
        data->CommitTempBuffer();
      }
      if (log) {
          std::stringstream msg;
          pid_t tid = syscall(SYS_gettid);
          msg << "### TID: " << tid << "  Write Particles (Old Translator) end : " << log->GetTime(); // NOLINT
          log->WriteToFile(msg.str());
      }
      return true;
    }

    /* Reads the removed particle data from the PhysicalDataInstances
     * "instances", limited by the corresponding global region calculated from
     * shifting the local region "region" by the offset "shift", into the local
     * region "region" of the PhysBAM particle container "particle_container".
     *
     * "positive" option specifies whether to work on positive particles or
     * negative particles.
     * "merge" option specifies whether to keep original particle data in
     * "particle_container".
     */
    bool ReadRemovedParticles(const GeometricRegion* region,
                              const int_dimension_t shift[3],
                              const PdiVector* instances,
                              ParticleContainer& particle_container,
                              const int_dimension_t kScale,
                              bool positive,
                              bool merge = false) {
      if (log) {
          std::stringstream msg;
          pid_t tid = syscall(SYS_gettid);
          msg << "### TID: " << tid << "  Read Removed Particles (Old Translator) start : " << log->GetTime(); // NOLINT
          log->WriteToFile(msg.str());
      }
      RemovedParticleArray* particles;
      if (positive) {
        particles = &particle_container.removed_positive_particles;
      } else {
        particles = &particle_container.removed_negative_particles;
      }

      // Checks whether the geometric region in the particle array is valid, and
      // clears corresponding buckets inside the geometric region if necessary.
      for (int z = region->z(); z <= region->z() + region->dz(); z++)
        for (int y = region->y(); y <= region->y() + region->dy(); y++)
          for (int x = region->x(); x <= region->x() + region->dx(); x++) {
            TV_INT bucket_index(x, y, z);
            if (!particles->Valid_Index(bucket_index)) {
              dbg(DBG_WARN, "Bucket index (%d, %d, %d) out of range.\n",
                            x, y, z);
              // Warning: might be too strict.
              if (log) {
                  std::stringstream msg;
                  pid_t tid = syscall(SYS_gettid);
                  msg << "### TID: " << tid << "  Read Removed Particles (Old Translator) end : " << log->GetTime(); // NOLINT
                  log->WriteToFile(msg.str());
              }
              return false;
            }
            if (!merge) {
              if ((*particles)(bucket_index)) {
                delete (*particles)(bucket_index);
                (*particles)(bucket_index) = NULL;
              }
            }
          }

      if (instances == NULL) {
        dbg(DBG_WARN, "Physical data instances are empty.\n");
        if (log) {
            std::stringstream msg;
            pid_t tid = syscall(SYS_gettid);
            msg << "### TID: " << tid << "  Read Removed Particles (Old Translator) end : " << log->GetTime(); // NOLINT
            log->WriteToFile(msg.str());
        }
        return false;
      }

      PdiVector::const_iterator iter = instances->begin();
      for (; iter != instances->end(); ++iter) {
        const PhysicalDataInstance* instance = *iter;
        PhysBAMData* data = static_cast<PhysBAMData*>(instance->data());
        RemovedParticleInternal* buffer =
            reinterpret_cast<RemovedParticleInternal*>(data->buffer());
        RemovedParticleInternal* buffer_end = buffer
            + static_cast<int>(data->size())
            / static_cast<int>(sizeof(RemovedParticleInternal));

        for (RemovedParticleInternal* p = buffer; p != buffer_end; ++p) {
          VECTOR_TYPE absolute_position;
          absolute_position.x = p->position[0];
          absolute_position.y = p->position[1];
          absolute_position.z = p->position[2];
          if (absolute_position.x >= region->x() + shift[0] &&
              absolute_position.x < region->x() + region->dx() + shift[0] &&
              absolute_position.y >= region->y() + shift[1] &&
              absolute_position.y < region->y() + region->dy() + shift[1] &&
              absolute_position.z >= region->z() + shift[2] &&
              absolute_position.z < region->z() + region->dz() + shift[2]) {
            TV_INT bucket_index(round(absolute_position.x - shift[0]),
                                round(absolute_position.y - shift[1]),
                                round(absolute_position.z - shift[2]));
            assert(particles->Valid_Index(bucket_index));
            // NOTE(By Chinmayee): Please comment out these changes and don't
            // delete them when pushing any updates, till we verify that the
            // code works correctly, and does not give any assertion failure or
            // seg fault on both Linux and Mac.
            if (!(*particles)(bucket_index)) {
              (*particles)(bucket_index) =
                  particle_container.Allocate_Particles(
                  particle_container.template_removed_particles);
            }
            RemovedParticleBucket* particle_bucket =
                (*particles)(bucket_index);

            // Note that Add_Particle traverses a linked list of particle
            // buckets, so it's O(N^2) time. Blech.
            int index = particle_container.Add_Particle(particle_bucket);
            particle_bucket->X(index) =
                (absolute_position - 1.0) / (float) kScale; // NOLINT
            particle_bucket->radius(index) = p->radius;
            particle_bucket->quantized_collision_distance(index) =
              p->quantized_collision_distance;
            if (particle_container.store_unique_particle_id) {
              PhysBAM::ARRAY_VIEW<int>* id = particle_bucket->array_collection->
                  template Get_Array<int>(PhysBAM::ATTRIBUTE_ID_ID);
              (*id)(index) = p->id;
            }
            particle_bucket->V(index) = VECTOR_TYPE(p->v[0],
                                                    p->v[1],
                                                    p->v[2]);
          }
        }  // End the loop for buffer.
      }
      if (log) {
          std::stringstream msg;
          pid_t tid = syscall(SYS_gettid);
          msg << "### TID: " << tid << "  Read Removed Particles (Old Translator) end : " << log->GetTime(); // NOLINT
          log->WriteToFile(msg.str());
      }
      return true;
    }

    /* Writes the removed particle data from PhysBAM particle container
     * "particle_container", limited the local region "region", into the
     * corresponding global region of physical instances "instances" after
     * performing the coordinate shifting specified by "shift".
     *
     * "positive" option specifies whether to work on positive particles or
     * negative particles.
     */
    // TODO(quhang) The similar optimization in WriteParticles can be put here.
    bool WriteRemovedParticles(const GeometricRegion* region,
                               const int_dimension_t shift[3],
                               PdiVector* instances,
                               ParticleContainer& particle_container,
                               const int_dimension_t kScale,
                               bool positive
                               ) {
      if (log) {
          std::stringstream msg;
          pid_t tid = syscall(SYS_gettid);
          msg << "### TID: " << tid << "  Write Removed Particles (Old Translator) start : " << log->GetTime(); // NOLINT
          log->WriteToFile(msg.str());
      }
      PdiVector::iterator iter = instances->begin();
      for (; iter != instances->end(); ++iter) {
        const PhysicalDataInstance* instance = *iter;
        PhysBAMData* data = static_cast<PhysBAMData*>(instance->data());
        data->ClearTempBuffer();
      }

      RemovedParticleArray* particles;
      if (positive) {
        particles = &particle_container.removed_positive_particles;
      } else {
        particles = &particle_container.removed_negative_particles;
      }

      // Loop through each particle bucket in the specified region.
      for (int z = region->z(); z <= region->z() + region->dz(); z++)
        for (int y = region->y(); y <= region->y() + region->dy(); y++)
          for (int x = region->x(); x <= region->x() + region->dx(); x++) {
            TV_INT bucket_index(x, y, z);
            if (!particles->Valid_Index(bucket_index)) {
              dbg(DBG_WARN, "Bucket index (%d, %d, %d) out of range.\n",
                            x, y, z);
              if (log) {
                  std::stringstream msg;
                  pid_t tid = syscall(SYS_gettid);
                  msg << "### TID: " << tid << "  Write Removed Particles (Old Translator) end : " << log->GetTime(); // NOLINT
                  log->WriteToFile(msg.str());
              }
              return false;
            }
            RemovedParticleBucket* particle_bucket = (*particles)(bucket_index);
            while (particle_bucket) {
              for (int i = 1; i <= particle_bucket->array_collection->Size();
                   i++) {
                VECTOR_TYPE particle_position = particle_bucket->X(i);
                VECTOR_TYPE absolute_position =
                    particle_position * (float) kScale + 1.0; // NOLINT
                PdiVector::iterator iter = instances->begin();
                // Iterate across instances, checking each one.
                for (; iter != instances->end(); ++iter) {
                  const PhysicalDataInstance* instance = *iter;
                  GeometricRegion* instanceRegion = instance->region();
                  // If it's inside the region of the physical data instance.
                  if (absolute_position.x >=
                          instanceRegion->x() &&
                      absolute_position.x <
                          (instanceRegion->x() + instanceRegion->dx()) &&
                      absolute_position.y >=
                          instanceRegion->y() &&
                      absolute_position.y <
                          (instanceRegion->y() + instanceRegion->dy()) &&
                      absolute_position.z >=
                          instanceRegion->z() &&
                      absolute_position.z <
                          (instanceRegion->z() + instanceRegion->dz())) {
                    RemovedParticleInternal particle_buffer;
                    particle_buffer.position[0] = absolute_position.x;
                    particle_buffer.position[1] = absolute_position.y;
                    particle_buffer.position[2] = absolute_position.z;
                    particle_buffer.radius = particle_bucket->radius(i);
                    particle_buffer.quantized_collision_distance =
                        particle_bucket->quantized_collision_distance(i);
                    if (particle_container.store_unique_particle_id) {
                      PhysBAM::ARRAY_VIEW<int>* id =
                          particle_bucket->array_collection->
                          template Get_Array<int>(PhysBAM::ATTRIBUTE_ID_ID);
                      particle_buffer.id = (*id)(i);
                    }
                    particle_buffer.v[0] = particle_bucket->V(i).x;
                    particle_buffer.v[1] = particle_bucket->V(i).y;
                    particle_buffer.v[2] = particle_bucket->V(i).z;
                    PhysBAMData* data =
                        static_cast<PhysBAMData*>(instance->data());
                    data->AddToTempBuffer(
                        reinterpret_cast<char*>(&particle_buffer),
                        sizeof(particle_buffer));
                  }
                }
              }
              particle_bucket = particle_bucket->next;
            }
          }

      // Now that we've copied particles into the temporary data buffers,
      // commit the results.
      iter = instances->begin();
      for (; iter != instances->end(); ++iter) {
        const PhysicalDataInstance* instance = *iter;
        PhysBAMData* data = static_cast<PhysBAMData*>(instance->data());
        data->CommitTempBuffer();
      }
      if (log) {
          std::stringstream msg;
          pid_t tid = syscall(SYS_gettid);
          msg << "### TID: " << tid << "  Write Removed Particles (Old Translator) end : " << log->GetTime(); // NOLINT
          log->WriteToFile(msg.str());
      }
      return true;
    }

    /* Read scalar array from PhysicalDataInstances specified by instances,
     * limited by the GeometricRegion specified by region, into the
     * ScalarArray specified by dest. This allocates a new scalar array. */
    template<typename T> typename PhysBAM::ARRAY<T, Int3Vector>* ReadScalarArray(
        const GeometricRegion* region,
        const int_dimension_t shift[3],
        const PdiVector* instances,
        typename PhysBAM::ARRAY<T, Int3Vector>* sa) {
      if (log) {
          std::stringstream msg;
          pid_t tid = syscall(SYS_gettid);
          msg << "### TID: " << tid << "  Read Scalar Array (Old Translator) start : " << log->GetTime(); // NOLINT
          log->WriteToFile(msg.str());
      }
      if (instances != NULL) {
        sa->hash_code = 0;
        PdiVector::const_iterator iter = instances->begin();
        for (; iter != instances->end(); ++iter) {
          const PhysicalDataInstance* inst = *iter;
          Dimension3Vector overlap = GetOverlapSize(inst->region(), region);

          if (HasOverlap(overlap)) {
            PhysBAMData* data = static_cast<PhysBAMData*>(inst->data());
            T* buffer  = reinterpret_cast<T*>(data->buffer());

            Dimension3Vector src  = GetOffset(inst->region(), region);
            Dimension3Vector dest = GetOffset(region, inst->region());

            for (int x = 0; x < overlap(X_COORD); x++) {
              for (int y = 0; y < overlap(Y_COORD); y++) {
                for (int z = 0; z < overlap(Z_COORD); z++) {
                  int source_x = x + src(X_COORD);
                  int source_y = y + src(Y_COORD);
                  int source_z = z + src(Z_COORD);
                  int source_index =
                      (source_x * (inst->region()->dy() * inst->region()->dz())) +
                      (source_y * (inst->region()->dz())) +
                      source_z;
                  int dest_x = x + dest(X_COORD) + region->x() - shift[0];
                  int dest_y = y + dest(Y_COORD) + region->y() - shift[1];
                  int dest_z = z + dest(Z_COORD) + region->z() - shift[2];
                  Int3Vector destination_index(dest_x, dest_y, dest_z);
                  assert(source_index < data->size() / (int) sizeof(T) && source_index >= 0); // NOLINT
                  (*sa)(destination_index) = buffer[source_index];
                }
              }
            }
          }
        }
      }
      if (log) {
          std::stringstream msg;
          pid_t tid = syscall(SYS_gettid);
          msg << "### TID: " << tid << "  Read Scalar Array (Old Translator) end : " << log->GetTime(); // NOLINT
          log->WriteToFile(msg.str());
      }
      return sa;
    }

    /* Helper function to specify the element type. Implicit template type
     * inference seems unreliable.
     */
    typename PhysBAM::ARRAY<float, Int3Vector>* ReadScalarArrayFloat(
        const GeometricRegion* region,
        const int_dimension_t shift[3],
        const PdiVector* instances,
        typename PhysBAM::ARRAY<float, Int3Vector>* sa) {
      return ReadScalarArray<float>(region, shift, instances, sa);
    }

    typename PhysBAM::ARRAY<int, Int3Vector>* ReadScalarArrayInt(
        const GeometricRegion* region,
        const int_dimension_t shift[3],
        const PdiVector* instances,
        typename PhysBAM::ARRAY<int, Int3Vector>* sa) {
      return ReadScalarArray<int>(region, shift, instances, sa);
    }

    typename PhysBAM::ARRAY<bool, Int3Vector>* ReadScalarArrayBool(
        const GeometricRegion* region,
        const int_dimension_t shift[3],
        const PdiVector* instances,
        typename PhysBAM::ARRAY<bool, Int3Vector>* sa) {
      return ReadScalarArray<bool>(region, shift, instances, sa);
    }

    /* Write scalar array data into PhysicalDataInstances specified by instances,
     * limited by the GeometricRegion region. This frees the physbam scalar array. */
    template<typename T> bool WriteScalarArray(
        const GeometricRegion* region,
        const int_dimension_t shift[3],
        PdiVector* instances,
        typename PhysBAM::ARRAY<T, Int3Vector>* sa) {
      if (log) {
          std::stringstream msg;
          pid_t tid = syscall(SYS_gettid);
          msg << "### TID: " << tid << "  Write Scalar Array (Old Translator) start : " << log->GetTime(); // NOLINT
          log->WriteToFile(msg.str());
      }
      if (sa->counts != Int3Vector(region->dx(), region->dy(), region->dz())) {
        dbg(DBG_WARN, "WARN: writing to a scalar array of a different size\n");
        Int3Vector cp = sa->counts;
        dbg(DBG_WARN, "WARN: physbam array has size %i, %i, %i\n", cp.x, cp.y, cp.z);
        dbg(DBG_WARN, "WARN: original array has size %llu, %llu, %llu\n", region->dx(), region->dy(), region->dz()); // NOLINT
      }

      if (instances != NULL) {
        PdiVector::iterator iter = instances->begin();

        for (; iter != instances->end(); ++iter) {
          const PhysicalDataInstance* inst = *iter;
          GeometricRegion *temp = inst->region();
          Dimension3Vector overlap = GetOverlapSize(temp, region);

          if (HasOverlap(overlap)) {
            PhysBAMData* data = static_cast<PhysBAMData*>(inst->data());
            T* buffer  = reinterpret_cast<T*>(data->buffer());

            Dimension3Vector src  = GetOffset(region, inst->region());
            Dimension3Vector dest = GetOffset(inst->region(), region);

            for (int x = 0; x < overlap(X_COORD); x++) {
              for (int y = 0; y < overlap(Y_COORD); y++) {
                for (int z = 0; z < overlap(Z_COORD); z++) {
                  int dest_x = x + dest(X_COORD);
                  int dest_y = y + dest(Y_COORD);
                  int dest_z = z + dest(Z_COORD);
                  int destination_index =
                      (dest_x * (inst->region()->dy() * inst->region()->dz())) +
                      (dest_y * (inst->region()->dz())) +
                      dest_z;
                  int source_x = x + src(X_COORD) + region->x() - shift[0];
                  int source_y = y + src(Y_COORD) + region->y() - shift[1];
                  int source_z = z + src(Z_COORD) + region->z() - shift[2];
                  Int3Vector source_index(source_x, source_y, source_z);
                  assert(destination_index < data->size() / (int) sizeof(T) && destination_index >= 0); // NOLINT
                  buffer[destination_index] = (*sa)(source_index);
                }
              }
            }
          }
        }
      }
      if (log) {
          std::stringstream msg;
          pid_t tid = syscall(SYS_gettid);
          msg << "### TID: " << tid << "  Write Scalar Array (Old Translator) end : " << log->GetTime(); // NOLINT
          log->WriteToFile(msg.str());
      }
      return true;
    }

    /* Helper function to specify the element type. Implicit template type
     * inference seems unreliable.
     */
    bool WriteScalarArrayFloat(
        const GeometricRegion* region,
        const int_dimension_t shift[3],
        PdiVector* instances,
        typename PhysBAM::ARRAY<float, Int3Vector>* sa) {
      return WriteScalarArray<float>(region, shift, instances, sa);
    }

    bool WriteScalarArrayInt(
        const GeometricRegion* region,
        const int_dimension_t shift[3],
        PdiVector* instances,
        typename PhysBAM::ARRAY<int, Int3Vector>* sa) {
      return WriteScalarArray<int>(region, shift, instances, sa);
    }

    bool WriteScalarArrayBool(
        const GeometricRegion* region,
        const int_dimension_t shift[3],
        PdiVector* instances,
        typename PhysBAM::ARRAY<bool, Int3Vector>* sa) {
      return WriteScalarArray<bool>(region, shift, instances, sa);
    }

    template<typename T> static void ReadCompressedScalarArray(
        const GeometricRegion &region,
        const int_dimension_t shift[3],
        PdiVector& read_set,
        PhysBAM::VECTOR_ND<T>* data,
        const int_dimension_t data_length,
        const PhysBAM::ARRAY<int, TV_INT>& index_data) {
      if (data->Size() != data_length) {
        data->Resize(data_length);
      }
      if (log) {
        std::stringstream msg;
        msg << "### Read Compressed Scalar Array (New Translator) start : "
            << log->GetTime();
        log->WriteToFile(msg.str());
      }
      if (read_set.empty()) {
        if (log) {
          std::stringstream msg;
          msg << "### Read Compressed Scalar Array (New Translator) end : "
              << log->GetTime();
          log->WriteToFile(msg.str());
        }
        return;
      }
      for (size_t i = 0; i < read_set.size(); ++i) {
        PhysBAMDataWithMeta* nimbus_data =
            dynamic_cast<PhysBAMDataWithMeta*>(read_set[i]->data());  // NOLINT
        assert(nimbus_data != NULL);
        GeometricRegion inter_region = GeometricRegion::GetIntersection(
            nimbus_data->region(), region);
        char* buffer = nimbus_data->buffer();
        if (buffer == NULL || nimbus_data->size() == 0) {
          continue;
        }
        assert(nimbus_data->has_meta_data());
        int_dimension_t elements =
            nimbus_data->meta_data_size() / 3 / sizeof(int_dimension_t);
        char* real_data_buffer = buffer + nimbus_data->meta_data_size();
        for (int_dimension_t rank = 0; rank < elements; ++rank) {
          int_dimension_t x = *reinterpret_cast<int_dimension_t*>(buffer);
          buffer += sizeof(int_dimension_t);
          int_dimension_t y = *reinterpret_cast<int_dimension_t*>(buffer);
          buffer += sizeof(int_dimension_t);
          int_dimension_t z = *reinterpret_cast<int_dimension_t*>(buffer);
          buffer += sizeof(int_dimension_t);
          T value = *reinterpret_cast<T*>(real_data_buffer);
          real_data_buffer += sizeof(T);
          if (x >= inter_region.x() &&
              x < inter_region.x() + inter_region.dx() &&
              y >= inter_region.y() &&
              y < inter_region.y() + inter_region.dy() &&
              z >= inter_region.z() &&
              z < inter_region.z() + inter_region.dz()) {
            int m_index = index_data(x - shift[0], y - shift[1], z - shift[2]);
            assert(m_index >= 1);
            (*data)(m_index) = value;
          }
        }
      }
      if (log) {
        std::stringstream msg;
        msg << "### Read Compressed Scalar Array (New Translator) end : "
            << log->GetTime();
        log->WriteToFile(msg.str());
      }
    }

    template<typename T> static void WriteCompressedScalarArray(
        const GeometricRegion &region,
        const int_dimension_t shift[3],
        PdiVector& write_set,
        const PhysBAM::VECTOR_ND<T>& data,
        const int_dimension_t data_length,
        const PhysBAM::ARRAY<int, TV_INT>& index_data) {
      if (log) {
        std::stringstream msg;
        msg << "### Write Compressed Scalar Array (New Translator) start : "
            << log->GetTime();
        log->WriteToFile(msg.str());
      }
      if (write_set.empty()) {
        if (log) {
          std::stringstream msg;
          msg << "### Write Compressed Scalar Array (New Translator) end : "
              << log->GetTime();
          log->WriteToFile(msg.str());
        }
        return;
      }
      for (size_t i = 0; i < write_set.size(); ++i) {
        PhysBAMDataWithMeta* nimbus_data =
            dynamic_cast<PhysBAMDataWithMeta*>(write_set[i]->data());  // NOLINT
        assert(nimbus_data != NULL);
        GeometricRegion inter_region = GeometricRegion::GetIntersection(
            nimbus_data->region(), region);
        if (!inter_region.NoneZeroArea()) {
          continue;
        }
        nimbus_data->ClearTempBuffer();
        std::list<T> buffer;
        for (int_dimension_t x = inter_region.x();
             x < inter_region.x() + inter_region.dx();
             ++x)
          for (int_dimension_t y = inter_region.y();
               y < inter_region.y() + inter_region.dy();
               ++y)
            for (int_dimension_t z = inter_region.z();
                 z < inter_region.z() + inter_region.dz();
                 ++z) {
              int m_index = index_data(TV_INT(
                      x - shift[0], y - shift[1], z - shift[2]));
              if (m_index >= 1) {
                nimbus_data->AddToTempBuffer(
                    reinterpret_cast<char*>(&x),
                    sizeof(x));
                nimbus_data->AddToTempBuffer(
                    reinterpret_cast<char*>(&y),
                    sizeof(y));
                nimbus_data->AddToTempBuffer(
                    reinterpret_cast<char*>(&z),
                    sizeof(z));
                buffer.push_back(data(m_index));
              }
            }
        nimbus_data->MarkMetaDataInTempBuffer();
        if (!buffer.empty()) {
          for (typename std::list<T>::iterator iter = buffer.begin();
               iter != buffer.end();
               ++iter) {
            T value = *iter;
            nimbus_data->AddToTempBuffer(
                reinterpret_cast<char*>(&value), sizeof(value));
          }
        }
        nimbus_data->CommitTempBuffer();
      }
      if (log) {
        std::stringstream msg;
        msg << "### Write Compressed Scalar Array (New Translator) end : "
            << log->GetTime();
        log->WriteToFile(msg.str());
      }
    }

  private:
    /* Return a vector describing what the offset of b
       within a, such that a.x + offset = b.x. If
       offset is negative, return 0. */
    virtual Dimension3Vector GetOffset(const GeometricRegion* a,
                                       const GeometricRegion* b) {
      Dimension3Vector result;

      // If source is > than dest, its offset is zero (it's contained),
      // otherwise the offset is the difference between the values.
      int_dimension_t x = b->x() - a->x();
      int_dimension_t y = b->y() - a->y();
      int_dimension_t z = b->z() - a->z();
      result(X_COORD) = (x >= 0)? x:0;
      result(Y_COORD) = (y >= 0)? y:0;
      result(Z_COORD) = (z >= 0)? z:0;

      return result;
    }

    virtual Dimension3Vector GetOverlapSize(const GeometricRegion* src,
                                            const GeometricRegion* dest) {
      Dimension3Vector result;

      int_dimension_t x_start = std::max(src->x(), dest->x());
      int_dimension_t x_end   = std::min(src->x() + src->dx(),
                                         dest->x() + dest->dx());
      int_dimension_t x_size = x_end - x_start;

      int_dimension_t y_start = std::max(src->y(), dest->y());
      int_dimension_t y_end   = std::min(src->y() + src->dy(),
                                         dest->y() + dest->dy());
      int_dimension_t y_size = y_end - y_start;

      int_dimension_t z_start = std::max(src->z(), dest->z());
      int_dimension_t z_end   = std::min(src->z() + src->dz(),
                                         dest->z() + dest->dz());
      int_dimension_t z_size = z_end - z_start;

      result(X_COORD) = (x_size >= 0)? x_size:0;
      result(Y_COORD) = (y_size >= 0)? y_size:0;
      result(Z_COORD) = (z_size >= 0)? z_size:0;

      return result;
    }

    virtual bool HasOverlap(Dimension3Vector overlapSize) {
      return (overlapSize(X_COORD) > 0 &&
              overlapSize(Y_COORD) > 0 &&
              overlapSize(Z_COORD) > 0);
    }
};

template class TranslatorPhysBAMOld<PhysBAM::VECTOR<float, 3> >;
template <class VECTOR_TYPE>
Log *TranslatorPhysBAMOld<VECTOR_TYPE>::log = NULL;
}  // namespace nimbus

#endif  // NIMBUS_SRC_DATA_PHYSBAM_TRANSLATOR_PHYSBAM_OLD_H_
