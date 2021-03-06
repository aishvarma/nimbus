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
  * Meta before set information used to keep track of immediate and indirect
  * before set of a job.
  *
  * Author: Omid Mashayekhi <omidm@stanford.edu>
  */

#ifndef NIMBUS_SRC_SCHEDULER_META_BEFORE_SET_H_
#define NIMBUS_SRC_SCHEDULER_META_BEFORE_SET_H_

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <list>
#include <utility>
#include "src/shared/nimbus_types.h"
#include "src/shared/dbg.h"

namespace nimbus {

  class MetaBeforeSet {
  public:
    typedef boost::unordered_set<job_id_t> Cache;
    typedef boost::unordered_map<job_id_t, boost::shared_ptr<MetaBeforeSet> > Table;

    MetaBeforeSet();

    MetaBeforeSet(const MetaBeforeSet& other);

    virtual ~MetaBeforeSet();

    Table table() const;
    const Table* table_p() const;
    Table* table_p();
    Cache positive_query() const;
    const Cache* positive_query_p() const;
    Cache* positive_query_p();
    Cache negative_query() const;
    const Cache* negative_query_p() const;
    Cache* negative_query_p();
    bool is_root() const;
    job_depth_t job_depth();

    void set_table(const Table& table);
    void set_positive_query(const Cache& positive_query);
    void set_negative_query(const Cache& negative_query);
    void set_is_root(bool flag);
    void set_job_depth(job_depth_t job_depth);

    MetaBeforeSet& operator= (const MetaBeforeSet& right);

    bool LookUpBeforeSetChain(job_id_t job_id, job_depth_t job_depth);

    void InvalidateNegativeQueryCache();

    void Clear();

  private:
    Table table_;
    Cache positive_query_;
    Cache negative_query_;
    bool is_root_;
    job_depth_t job_depth_;
    boost::mutex mutex_;
  };

}  // namespace nimbus

#endif  // NIMBUS_SRC_SCHEDULER_META_BEFORE_SET_H_
