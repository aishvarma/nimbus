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
 * Grid array is an array indexed by 3-dimension tuples. It is internally
 * serialzed as ARRAY_VIEW in PhysBAM. Given its complexity, this class takes no
 * responsibility to manage memory for ARRAY_VIEW, which should be instead done
 * by the caller.
 *
 * Author: Hang Qu <quhang@stanford.edu>
 */

#include "src/data/physbam/physbam_data.h"
#include "src/shared/nimbus.h"

#include "applications/physbam/water//projection/data_raw_grid_array.h"

namespace application {

DataRawGridArray::DataRawGridArray(std::string name) {
  set_name(name);
}

nimbus::Data* DataRawGridArray::Clone() {
  return (new DataRawGridArray(name()));
}

bool DataRawGridArray::SaveToNimbus(
    const PhysBAM::ARRAY<T, TV_INT>& array_input) {
  Header header;
  header.n = array_input.counts.Product();
  ClearTempBuffer();
  AddToTempBuffer(reinterpret_cast<char*>(&header), sizeof(header));
  AddToTempBuffer(
      const_cast<char*>(
          reinterpret_cast<const char*>(array_input.array.Get_Array_Pointer())),
      header.n * sizeof(T));
  CommitTempBuffer();
  return true;
}

bool DataRawGridArray::LoadFromNimbus(PhysBAM::ARRAY<T, TV_INT>* array) {
  assert(array != NULL);
  char* pointer = buffer();
  assert(pointer != NULL);
  const Header &header = *(reinterpret_cast<const Header*>(pointer));
  // Notice, Nimbus assumes the array is already initialized.
  assert(array->counts.Product() == header.n);
  pointer += sizeof(Header);
  memcpy(array->array.Get_Array_Pointer(), pointer, header.n * sizeof(T));
  array->hash_code = 0;
  return true;
}

float DataRawGridArray::FloatingHash() {
  const T *b = (int *)buffer();
  size_t s = size();
  T sum = 0;
  for (size_t i = 0; i < (s/sizeof(T)); ++i) {
    sum += b[i];
  }
  return(float(sum));
}

} // namespace application
