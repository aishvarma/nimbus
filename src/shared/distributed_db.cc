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
  * Distributed DB class implements a service that is used to save the state
  * required for checkpoint creation in order to provide fault tolerance.
  * 
  * It is basically a key-value store. Upon saving a value with a specific key,
  * the service returns a handle that can be used later to retrieve the value.
  * The handle is overloaded version of the key such that any node in the
  * cluster could find the primary node that saved the data and retrieve it
  * form that node. This way even if a worker is down, other nodes could access
  * the saved state from the non-volatile memory that worker save the data on.
  *
  * Currently each node of the distributed db has a leveldb module to save the
  * value on local disk. The handle interface makes it easy to change the
  * implementation anytime in the future to use other options like AWS S3.
  * For LevelDB implementation handle has the following format:
  * 
  * handle: <node_ip_address>-<leveldb_root>-<key>
  *
  * Author: Omid Mashayekhi <omidm@stanford.edu>
  */


#include "src/shared/distributed_db.h"

using namespace nimbus; // NOLINT

DistributedDB::DistributedDB() {
  initialized_ = false;
}

DistributedDB::~DistributedDB() {
}


void DistributedDB::Initialize(const std::string& ip_address,
                               const worker_id_t& worker_id) {
  boost::recursive_mutex::scoped_lock lock(mutex_);
  ip_address_ = ip_address;
  worker_id_ = worker_id;
  path_ = exec("pwd") + "/_db_"+ int2string(worker_id) + "_" + exec("date +%T") + "/";
  exec(("rm -rf " + path_).c_str());
  exec(("mkdir -p " + path_).c_str());
  initialized_ = true;
  return;
}

bool DistributedDB::Put(const std::string& key,
                        const std::string& value,
                        const checkpoint_id_t& checkpoint_id,
                        std::string *handle) {
  boost::recursive_mutex::scoped_lock lock(mutex_);
  std::string db_root =
    path_ + int2string(checkpoint_id) + "_" + int2string(worker_id_);

  leveldb::DB *db = GetDB(ip_address_, db_root);
  db->Put(leveldb::WriteOptions(), key, value);


  Handle h(ip_address_, db_root, key);
  return h.Serialize(handle);
}

bool DistributedDB::Get(const std::string& handle,
                        std::string *value) {
  boost::recursive_mutex::scoped_lock lock(mutex_);
  Handle h;
  h.Parse(handle);

  leveldb::DB *db = GetDB(h.ip_address(), h.db_root());
  db->Get(leveldb::ReadOptions(), h.key(), value);
  return true;
}

bool DistributedDB::RemoveCheckpoint(checkpoint_id_t checkpoint_id) {
  boost::recursive_mutex::scoped_lock lock(mutex_);
  return false;
}


leveldb::DB* DistributedDB::GetDB(const std::string& ip_address,
                                  const std::string& db_root) {
  boost::recursive_mutex::scoped_lock lock(mutex_);
  Map::iterator iter = db_map_.find(db_root);
  if (iter != db_map_.end()) {
    return iter->second;
  }

  std::string path = db_root.substr(0, db_root.find_last_of("/") + 1);

  bool create;
  std::string new_db_root;

  if (path == path_) {
    create = true;
    new_db_root = db_root;
    dbg(DBG_SCHED, "Creating new db locally ...\n");
  } else {
    create = false;
    RetrieveDBFromOtherNode(ip_address, db_root, &new_db_root);
    dbg(DBG_SCHED, "Getting db from other node ...\n");
  }

  leveldb::DB* db;
  leveldb::Options options;
  options.create_if_missing = create;
  leveldb::Status status = leveldb::DB::Open(options, new_db_root, &db);
  assert(status.ok());

  db_map_[db_root] = db;
  return db;
}

bool DistributedDB::RetrieveDBFromOtherNode(const std::string& ip_address,
                                            const std::string& db_root,
                                            std::string *new_db_root) {
  boost::recursive_mutex::scoped_lock lock(mutex_);
  /* 
     1. generate rsa key pair and add to nimbus repository.

     2. Add the script to do the following:
      a. update ~/.ssh/authorized_keys to have the public key. 

   */

  std::string name = db_root.substr(db_root.find_last_of("/") + 1);
  *new_db_root = path_ + name;

  if (ip_address == ip_address_) {
    std::string command = "cp -r " + db_root + " " + path_;
    exec(command.c_str());
    dbg(DBG_SCHED, "Copied db locally.\n");

  } else {
    std::string command = "scp -r -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -i ";
    command += NIMBUS_LEVELDB_PRIVATE_KEY;
    command += " " + ip_address + ":" + db_root;
    command += " " + path_;
    exec(command.c_str());
    dbg(DBG_SCHED, "Copied db remotely.\n");
  }

  return true;
}

DistributedDB::Handle::Handle() {
}

DistributedDB::Handle::Handle(const std::string& ip_address,
                              const std::string& db_root,
                              const std::string& key) {
  ip_address_ = ip_address;
  db_root_ = db_root;
  key_ = key;
}

DistributedDB::Handle::~Handle() {
}

std::string DistributedDB::Handle::ip_address() {
  return ip_address_;
}

std::string DistributedDB::Handle::db_root() {
  return db_root_;
}

std::string DistributedDB::Handle::key() {
  return key_;
}

bool DistributedDB::Handle::Parse(const std::string& handle) {
  DBHandlePBuf buf;
  bool result = buf.ParseFromString(handle);

  if (!result) {
    dbg(DBG_ERROR, "ERROR: Failed to parse db handle.\n");
    return false;
  } else {
    ip_address_ = buf.ip_address();
    db_root_ = buf.db_root();
    key_ = buf.key();
    return true;
  }
}

bool DistributedDB::Handle::Serialize(std::string *result) {
  DBHandlePBuf buf;
  buf.set_ip_address(ip_address_);
  buf.set_db_root(db_root_);
  buf.set_key(key_);

  if (!buf.SerializeToString(result)) {
    dbg(DBG_ERROR, "ERROR: Failed to serilaize db handle.\n");
    return false;
  } else {
    return true;
  }
}

