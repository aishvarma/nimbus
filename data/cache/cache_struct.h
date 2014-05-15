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
 * A CacheStruct is an application object corresponding to a set of nimbus
 * variables cached by the cache manager.
 *
 * Author: Chinmayee Shah <chshah@stanford.edu>
 */

#ifndef NIMBUS_DATA_CACHE_CACHE_STRUCT_H_
#define NIMBUS_DATA_CACHE_CACHE_STRUCT_H_

#include <map>
#include <set>
#include <string>
#include <vector>

#include "data/cache/utils.h"
#include "shared/geometric_region.h"
#include "shared/nimbus_types.h"

namespace nimbus {

typedef int type_id_t;
class Data;
typedef std::vector<Data *> DataArray;
typedef std::set<Data *> DataSet;

/**
 * \class CacheStruct
 * \details Application object corresponding to a set of nimbus variables cached
 * by the cache manager. A CacheStruct can have num_variables number of
 * different nimbus variables, and must be defined over a geometric region.
 */
class CacheStruct {
    public:
        /**
         * \brief Creates a CacheStruct
         * \param num_variables indicates number of different application
         * variables that the CacheStruct instance contains
         * \return Constructed CacheStruct instance
         */
        explicit CacheStruct(size_t num_variables);

        /**
         * \brief Makes this instance a prototype
         */
         void MakePrototype();

        /**
         * \brief Creates a new CacheStruct instance using current instance
         * parameters
         * \param struct_region specifies the spatial domain of the CacheStruct
         * instance
         * \return Returns a pointer to the newly allocated CacheStruct instance
         */
        virtual CacheStruct *CreateNew(const GeometricRegion &struct_region) const;

        /**
         * \brief Updates CacheStruct with data from read_sets - performs update
         * only for those physical data that have changed since the CacheStruct
         * instance was created
         * \param var_type is a list of type_ids corresponding to nimbus variables
         * \param read_sets is a list of data arrays corresponding to nimbus variables
         * \param read_region is the geometric region to read
         */
        void UpdateCache(const std::vector<type_id_t> var_type,
                         const std::vector<DataArray *> read_sets,
                         const GeometricRegion &read_region);

        /**
         * \brief Indicates that write to CacheStruct instance is complete
         * \param write_region is the geometric region to write
         */
        void MarkWritten(const GeometricRegion &write_region);

        /**
         * \brief Flushes data from cache, removes corresponding dirty data
         * mapping
         * \param d is data to flush to
         */
        void FlushToData(Data *d);

        /**
         * \brief Acquires access to CacheStruct instance
         * \param access can be EXCLUSIVE or SHARED
         */
        void AcquireAccess(CacheAccess access);

        /**
         * \brief Releases access to CacheStruct instance
         */
        void ReleaseAccess();

        /**
         * \brief Checks if CacheStruct instance is available for use in access
         * mode
         * \param access denotes the mode (EXCLUSIVE/ SHARED) that application
         * wants
         * \return Boolean indicating if the instance is available
         */
        bool IsAvailable(CacheAccess access) const;

        /**
         * \brief Calculates distance of a CacheStruct, from the given list of
         * read_sets. This distance indicates the cost of reconstruction if
         * this CacheStruct instance is used.
         * \param var_type is a list of type_ids corresponding to nimbus variables
         * \param read_sets is a list of data arrays corresponding to nimbus variables
         * \return Returns distance (cost)
         */
        cache::distance_t GetDistance(const std::vector<type_id_t> var_type,
                                      const std::vector<DataArray *> read_sets) const;

        /**
         * \brief Sets up mappings for data in read_sets
         * \param var_type is a list of type_ids corresponding to nimbus variables
         * \param read_sets is a list of data arrays corresponding to nimbus variables
         */
        void SetUpRead(const std::vector<type_id_t> var_type,
                       const std::vector<DataArray *> read_sets);

        /**
         * \brief Sets up mappings for data in write_sets
         * \param var_type is a list of type_ids corresponding to nimbus variables
         * \param read_sets is a list of data arrays corresponding to nimbus variables
         */
        void SetUpWrite(const DataArray &write_set);

        /**
         * \brief Sets up mapping between data and CacheStruct instance
         * \param d denotes the data to map to
         */
        void SetUpData(Data *d);

        /**
         * \brief Removes mapping between data and CacheStruct instance
         * \param d denotes the data to unmap
         */
        void UnsetData(Data *d);

        /**
         * \brief Accessor for struct_region_ member
         * \return Instance's struct_region_, of type GeometricRegion
         */
        GeometricRegion struct_region() const;

        /**
         * \brief Setter for struct_region_ member
         * \param struct_region, of type GeometricRegion
         */
        void set_struct_region(const GeometricRegion &struct_region);

    private:
        /**
         * \brief Flushes data from cache to data in flush_sets (immediately)
         * \param var_type is a list of type_ids corresponding to nimbus variables
         * \param flush_sets is a list of data arrays corresponding to nimbus variables
         */
        void FlushCache(const std::vector<type_id_t> var_type,
                        const std::vector<DataArray *> read_sets);

        /**
         * \brief Setter for id_ member
         * \param id, of type size_t
         */
        void set_id(size_t id);

        // prototype information
        static size_t ids_allocated_;
        size_t id_;

        // number of nimbus variables
        size_t num_variables_;

        // access information
        CacheAccess access_;
        int users_;

        // read/ write/ struct region information
        GeometricRegion struct_region_;
        GeometricRegion write_region_;

        // cache-data mappings
        typedef std::map<GeometricRegion,
                         Data *,
                         cache::GRComparisonType> DMap;
        std::vector<DMap *> data_maps_;
        std::vector<DataSet *> write_backs_;

    protected:
        /**
         * \brief Reads data from read_sets into CacheStruct instance
         * \param var_type is a list of type_ids corresponding to nimbus variables
         * \param read_sets is a list of data arrays corresponding to nimbus variables
         * \param read_region is the geometric region to read
         */
        virtual void ReadToCache(const std::vector<type_id_t> var_type,
                                 const std::vector<DataArray *> read_sets,
                                 const GeometricRegion &read_region);

        /**
         * \brief Writes data from CacheStruct instance to write_sets
         * \param var_type is a list of type_ids corresponding to nimbus variables
         * \param write_sets is a list of data arrays corresponding to nimbus variables
         * \param write_region is the geometric region to be write
         */
        virtual void WriteFromCache(const std::vector<type_id_t> var_type,
                                    const std::vector<DataArray *> read_sets,
                                    const GeometricRegion &write_region);
};  // class CacheStruct

typedef std::vector<CacheStruct *> CacheStructs;

}  // namespace nimbus

#endif  // NIMBUS_DATA_CACHE_CACHE_STRUCT_H_
