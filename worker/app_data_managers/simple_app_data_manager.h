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
 * SimpleAppDataManager implements application data manager with simple_app_data caching of
 * application data across jobs.
 *
 * Author: Chinmayee Shah <chshah@stanford.edu>
 */

#ifndef NIMBUS_WORKER_APP_DATA_MANAGERS_SIMPLE_APP_DATA_MANAGER_H_
#define NIMBUS_WORKER_APP_DATA_MANAGERS_SIMPLE_APP_DATA_MANAGER_H_

#include <cstdio>
#include <sstream>  // NOLINT
#include <vector>
#include <string>

#include "data/app_data/app_data_defs.h"
#include "data/app_data/app_struct.h"
#include "data/app_data/app_var.h"
#include "worker/app_data_manager.h"

namespace nimbus {

class Data;
typedef std::vector<Data *> DataArray;
class GeometricRegion;

/**
 * \class SimpleAppDataManager
 * \details SimpleAppDataManager implements application data manager with simple_app_data
 * caching of application data across jobs.
 * Internally, SimpleAppDataManager contains a two level map - the
 * first level key is a prototype id, and the second level key is an
 * application object geometric region. Together, these keys map to a list of
 * AppVars or AppStructs.
 */
class SimpleAppDataManager : public AppDataManager {
    public:
        /**
         * \brief Creates a SimpleAppDataManager instance
         * \return Constructed SimpleAppDataManager instance
         */
        SimpleAppDataManager();

        /**
         * Destructor
         */
        ~SimpleAppDataManager();

        /**
         * \brief Informs application data manager that access to app_object is
         * no longer needed
         * \param app_object specified the application object to release
         */
        virtual void ReleaseAccess(AppObject* simple_app_data_object);

        /**
         * \brief Does not do anything
         * \param app_var is the application variable to write from
         * \param write_set is the set of nimbus objects to write to
         */
        virtual void WriteImmediately(AppVar *app_var, const DataArray &write_set);

        /**
         * \brief Does not do anything
         * \param app_struct is the application struct to write from
         * \param var_type specifies the type of variables in the struct that
         * are to be written, corresponding to write_sets
         * \param write_sets is the set of nimbus objects to write to
         */
        virtual void WriteImmediately(AppStruct *app_struct,
                              const std::vector<app_data::type_id_t> &var_type,
                              const std::vector<DataArray> &write_sets);

        /**
         * \brief Does not do anything
         * \param Data d to 
         */
        virtual void SyncData(Data *d);

        /**
         * \brief Does not do anything
         * \param Data d
         */
        virtual void InvalidateMappings(Data *d);

        /**
         * \brief Sets log file names for application data manager
         * \param Worker id, to be used in file names
         */
        virtual void SetLogNames(std::string wid_str);

    protected:
        /**
         * \brief Requests an AppVar instance of type prototype
         * \param read_set specifies the data that should be read in the
         * AppVar instance
         * \param read_region indicates read region
         * \param write_set specifies the data that should be marked as being
         * written
         * \param write_region indicates write region
         * \param prototype represents the application object type
         * \param region is the application object region
         * \access indicates whether application object access should be
         * EXCLUSIVE or SHARED, and is ignored when there is no caching
         * \return A pointer to AppVar instance that application can use
         */
        virtual AppVar *GetAppVarV(const DataArray &read_set,
                                   const GeometricRegion &read_region,
                                   const DataArray &write_set,
                                   const GeometricRegion &write_region,
                                   const AppVar &prototype,
                                   const GeometricRegion &region,
                                   app_data::Access access,
                                   void (*aux)(AppVar*, void*),
                                   void* aux_data);

        /**
         * \brief Requests an AppStruct instance of type prototype
         * \param var_type specifies the application variable types for the
         * lists in read_sets/ write_sets
         * \param read_sets specifies the data that should be read in the
         * AppVar instance
         * \param read_region indicates read region
         * \param write_sets specifies the data that should be marked as being
         * written
         * \param write_region indicates write region
         * \param prototype represents the application object type
         * \param region is the application object region
         * \access indicates whether application object access should be
         * EXCLUSIVE or SHARED
         * \return A pointer to AppStruct instance that application can use
         */
        virtual AppStruct *GetAppStructV(const std::vector<app_data::type_id_t> &var_type,
                                         const std::vector<DataArray> &read_sets,
                                         const GeometricRegion &read_region,
                                         const std::vector<DataArray> &write_sets,
                                         const GeometricRegion &write_region,
                                         const AppStruct &prototype,
                                         const GeometricRegion &region,
                                         app_data::Access access);

    private:
};  // class SimpleAppDataManager
}  // namespace nimbus

#endif  // NIMBUS_WORKER_APP_DATA_MANAGERS_SIMPLE_APP_DATA_MANAGER_H_
