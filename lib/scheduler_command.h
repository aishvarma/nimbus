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
  * Object representation of a scheduler command. Used by workers to
  * send commands to server and server to send commands down to workers.
  *
  * Author: Philip Levis <pal@cs.stanford.edu>
  */

#ifndef NIMBUS_LIB_SCHEDULER_COMMAND_H_
#define NIMBUS_LIB_SCHEDULER_COMMAND_H_

#include <sstream> // NOLINT
#include <string>
#include <vector>
#include <map>
#include <set>


class IDSet {
  public:
    IDSet();
    explicit IDSet(std::string s);
    ~IDSet();

    std::string toString();
    void insert(int n);
    void clear();
    int size();

    typedef std::set<int>::iterator IDSetIter;

  // private:
    std::set<int> set;
};

class CommandParameter {
  public:
    CommandParameter();
    explicit CommandParameter(std::string parameter);
    CommandParameter(std::string tag, std::string arg, const IDSet& set);
    virtual ~CommandParameter();

    std::string toString();
    std::string getTag();
    std::string getArg();
    IDSet getIDSet();

  private:
    std::string tag;
    std::string arg;
    IDSet set;
};

typedef std::map<std::string, CommandParameter> CommandParameterList;

class SchedulerCommand {
  public:
    SchedulerCommand();
    explicit SchedulerCommand(std::string command);
    SchedulerCommand(std::string name,
        const CommandParameterList& parameters);
    virtual ~SchedulerCommand();

    virtual void addParameter(CommandParameter cm);
    virtual std::string toString();
    virtual std::string getName();
    virtual CommandParameterList getParameters();

  private:
    std::string name;
    CommandParameterList parameters;
};

#endif  // NIMBUS_LIB_SCHEDULER_COMMAND_H_
