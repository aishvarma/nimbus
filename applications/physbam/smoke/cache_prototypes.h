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
 * Author: Chinmayee Shah <chshah@stanford.edu>
 * Modifier for smoke: Andrew Lim <alim16@stanford.edu> 
 */

#ifndef NIMBUS_APPLICATION_SMOKE_CACHE_PROTOTYPES_H_
#define NIMBUS_APPLICATION_SMOKE_CACHE_PROTOTYPES_H_

#include "application/smoke/cache_data_include.h"
#include "application/smoke/parameters.h"

namespace application {

extern CacheFaceArray<T> kCacheFaceVel;
extern CacheFaceArray<T> kCacheFaceVelGhost;
extern CacheFaceArray<bool> kCachePsiN;

extern CacheScalarArray<T> kCacheDensity;
extern CacheScalarArray<T> kCacheDensityGhost;

extern CacheScalarArray<bool> kCachePsiD;

// Varibales for projection.
extern CacheScalarArray<T> kCachePressure;
extern CacheScalarArray<int> kCacheColors;
extern CacheScalarArray<T> kCacheDivergence;
extern CacheRawGridArray kCacheIndexC2M;

extern CacheSparseMatrix kCacheSparseMatrixA;
extern CacheSparseMatrix kCacheSparseMatrixC;

extern CacheArrayM2C kCacheArrayM2C;

extern CacheCompressedScalarArray<float> kCacheMetaP;

extern CacheVector kCacheVectorB;
extern CacheVector kCacheVectorPressure;
extern CacheVector kCacheVectorZ;
extern CacheVector kCacheVectorTemp;

} // namespace application

#endif // NIMBUS_APPLICATION_SMOKE_CACHE_PROTOTYPES_H_
