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
 * This file defines the name of jobs that will be used for registration and
 * spawning the jobs.
 *
 * Author: Omid Mashayekhi <omidm@stanford.edu>
 */

#ifndef NIMBUS_APPLICATION_WATER_MULTIPLE_JOB_NAMES_H_
#define NIMBUS_APPLICATION_WATER_MULTIPLE_JOB_NAMES_H_

#include "applications/physbam/water//parameters.h"

#define MAIN "main"
#define INITIALIZE "initialize"
#define LOOP_FRAME "loop_frame"
#define LOOP_ITERATION "__MARK_STAT_loop_iteration"
#define LOOP_ITERATION_PART_TWO "loop_iteration_part_two"
#define WRITE_OUTPUT "write_output"
#define RESEED_PARTICLES "reseed_particles"
#define BARRIER_JOB "barrier_job"

#define CALCULATE_DT "calculate_dt"
#define UPDATE_GHOST_VELOCITIES "update_ghost_velocities"
#define EXTRAPOLATE_PHI "extrapolate_phi"
#define ADVECT_PHI "advect_phi"
#define STEP_PARTICLES "step_particles"
#define ADVECT_REMOVED_PARTICLES "advect_removed_particles"
#define ADVECT_V "advect_v"
#define APPLY_FORCES "apply_forces"
#define MODIFY_LEVELSET_PART_ONE "modify_levelset_part_one"
#define MODIFY_LEVELSET_PART_TWO "modify_levelset_part_two"
#define MAKE_SIGNED_DISTANCE "make_signed_distance"
#define ADJUST_PHI "adjust_phi"
#define DELETE_PARTICLES "delete_particles"
#define REINCORPORATE_PARTICLES "reincorporate_particles"
#define PROJECTION_MAIN "projection_main"
#define PROJECTION_TRANSFORM_PRESSURE "projection_transform_pressure"
#define PROJECTION_CALCULATE_BOUNDARY_CONDITION_PART_ONE "projection_calculate_boundary_condition_part_one"
#define PROJECTION_CALCULATE_BOUNDARY_CONDITION_PART_TWO "projection_calculate_boundary_condition_part_two"
#define PROJECTION_CONSTRUCT_MATRIX "projection_construct_matrix"
#define PROJECTION_WRAPUP "projection_wrapup"
#define EXTRAPOLATION "extrapolation"
#define SYNCHRONIZE_PARTICLES "synchronize_particles"

#define PROJECTION_GLOBAL_INITIALIZE "projection_global_initialize"
#define PROJECTION_LOCAL_INITIALIZE "projection_local_initialize"
#define PROJECTION_LOOP_ITERATION "projection_loop_iteration"
#define PROJECTION_LOOP_BOTTLENECK "projection_loop_bottleneck"
#define PROJECTION_STEP_ONE "projection_step_one"
#define PROJECTION_REDUCE_RHO "projection_reduce_rho"
#define PROJECTION_STEP_TWO "projection_step_two"
#define PROJECTION_STEP_THREE "projection_step_three"
#define PROJECTION_REDUCE_ALPHA "projection_reduce_alpha"
#define PROJECTION_STEP_FOUR "projection_step_four"

#endif  // NIMBUS_APPLICATION_WATER_MULTIPLE_JOB_NAMES_H_
