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

/***********************************************************************
 * AUTHOR: Philip Levis <pal>
 *   FILE: .//physical_data.cc
 *   DATE: Fri Nov  1 10:41:39 2013
 *  DESCR:
 ***********************************************************************/
#include "scheduler/physical_data.h"

namespace nimbus {
/**
 * \fn nimbus::PhysicalData::PhysicalData(data_id_t id,
                                   worker_id_t worker)
 * \brief Brief description.
 * \param id
 * \param worker
 * \return
*/
nimbus::PhysicalData::PhysicalData(data_id_t i,
                                   worker_id_t w) {
  id_ = i;
  worker_ = w;
  version_ = 0;
}


/**
 * \fn nimbus::PhysicalData::PhysicalData(data_id_t id,
                                   worker_id_t worker,
                                   data_version_t version)
 * \brief Brief description.
 * \param id
 * \param worker
 * \param version
 * \return
*/
nimbus::PhysicalData::PhysicalData(data_id_t i,
                                   worker_id_t w,
                                   data_version_t v) {
  id_ = i;
  worker_ = w;
  version_ = v;
}


/**
 * \fn nimbus::PhysicalData::~PhysicalData()
 * \brief Brief description.
 * \return
*/
nimbus::PhysicalData::~PhysicalData() {}


/**
 * \fn data_id_t nimbus::PhysicalData::id()
 * \brief Brief description.
 * \return
*/
data_id_t nimbus::PhysicalData::id() {
  return id_;
}


/**
 * \fn worker_id_t nimbus::PhysicalData::worker()
 * \brief Brief description.
 * \return
*/
worker_id_t nimbus::PhysicalData::worker() {
  return worker_;
}


/**
 * \fn data_version_t nimbus::PhysicalData::version()
 * \brief Brief description.
 * \return
*/
data_version_t nimbus::PhysicalData::version() {
  return version_;
}


/**
 * \fn void nimbus::PhysicalData::set_version(data_version_t v)
 * \brief Brief description.
 * \param v
 * \return
*/
void nimbus::PhysicalData::set_version(data_version_t v) {
  version_ = v;
}

}  // namespace nimbus
