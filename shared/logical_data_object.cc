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
 *   FILE: .//logical_data_object.cc
 *   DATE: Fri Oct 25 12:15:33 2013
 *  DESCR:
 ***********************************************************************/
#include "shared/logical_data_object.h"
#include "shared/dbg.h"

namespace nimbus {
/**
 * \fn nimbus::LogicalDataObject::LogicalDataObject(logical_data_id_t id,
                                             std::string variable,
                                             GeometricRegion *region)
 * \brief Brief description.
 * \param id
 * \param variable
 * \param region
 * \return
*/
LogicalDataObject::LogicalDataObject(logical_data_id_t id,
                                     std::string variable,
                                     GeometricRegion *region) {
  id_ = id;
  variable_ = variable;
  region_ = region;
}

LogicalDataObject::LogicalDataObject(std::istream* is) {
  LdoMessage msg;
  msg.ParseFromIstream(is);
  id_ = msg.data_id();
  variable_ = msg.variable();
  region_ = new GeometricRegion(&msg.region());
}

LogicalDataObject::LogicalDataObject(const std::string& data) {
  LdoMessage msg;
  msg.ParseFromString(data);
  id_ = msg.data_id();
  variable_ = msg.variable();
  region_ = new GeometricRegion(&msg.region());
}

/**
 * \fn nimbus::LogicalDataObject::~LogicalDataObject()
 * \brief Brief description.
 * \return
*/
LogicalDataObject::~LogicalDataObject() {
  dbg(DBG_DATA_OBJECTS|DBG_MEMORY, "Deleting geo region %llu: 0x%x\n", id_, region_);
  delete region_;
}


/**
 * \fn logical_data_id_t nimbus::LogicalDataObject::id()
 * \brief Brief description.
 * \return
*/
logical_data_id_t LogicalDataObject::id() const {
  return id_;
}


/**
 * \fn std::string nimbus::LogicalDataObject::variable()
 * \brief Brief description.
 * \return
*/
std::string LogicalDataObject::variable() const {
  return variable_;
}


/**
 * \fn GeometricRegion * nimbus::LogicalDataObject::region()
 * \brief Brief description.
 * \return
*/
GeometricRegion * LogicalDataObject::region() const {
  return region_;
}

bool LogicalDataObject::Serialize(std::ostream* os) {
  LdoMessage m;
  FillInMessage(&m);
  return m.SerializeToOstream(os);
}

bool LogicalDataObject::SerializeToString(std::string* output) {
  LdoMessage m;
  FillInMessage(&m);
  return m.SerializeToString(output);
}

void LogicalDataObject::FillInMessage(LdoMessage* msg) {
  msg->set_data_id(id_);
  msg->set_variable(variable_);
  region_->FillInMessage(msg->mutable_region());
}

}  // namespace nimbus
