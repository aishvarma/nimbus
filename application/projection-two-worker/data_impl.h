/*
 * Data specification of PhysBAM projection.
 * Cannot be written in a worse way. Rewrite it soon.
 * Use protocol buffer. Add access method.
 */

#ifndef __DATA_IMPL__
#define __DATA_IMPL__

#include "shared/nimbus.h"
#include "worker/data.h"

// Data structure to describe partial norm.
class PartialNorm : public Data {
 public:
  explicit PartialNorm(double norm) {norm_ = norm;}
  virtual ~PartialNorm() {}
  virtual void Create() {}
  virtual void Destroy() {}
  virtual Data* Clone() {
    return new PartialNorm(0);
  }
  virtual void Copy(Data* from) {
    PartialNorm* d = reinterpret_cast<PartialNorm*>(from);
    norm_ = d->norm_;
  }
  virtual bool Serialize(SerializedData* ser_data) {
    double* buffer = new double;
    *buffer = norm_;
    ser_data->set_data_ptr((char*)buffer);
    ser_data->set_size(sizeof(*buffer));
    return true;
  }
  virtual bool DeSerialize(const SerializedData& ser_data, Data** result) {
    double temp =  *(reinterpret_cast<double*>(ser_data.data_ptr_raw()));
    PartialNorm* partial_norm = new PartialNorm(temp);
    *result = partial_norm;
    return true;
  }
  double norm_;
};
#endif