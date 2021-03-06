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
  * Nimbus log interface.
  *
  * Author: Hang Qu <quhang@stanford.edu>
  */

#ifndef NIMBUS_SRC_SHARED_FAST_LOG_H_
#define NIMBUS_SRC_SHARED_FAST_LOG_H_

#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>  // for syscal in mac and debian

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#include <string.h>
#include <cassert>
#include <cstdio>
#include <map>
#include <utility>

namespace nimbus {
namespace timer {


typedef int64_t nimbus_ttimer_level;
extern nimbus_ttimer_level ttimer_level;

/* In order to add a new timer type, two places need to be editted:
 *   (1) Add the timer as an enumeration type in "enum TimerType";
 *   (2) Edit the "TimerName" function in "fast_log.cc" so that the logging
 *       system can print out a useful name for the added timer type.
 */
enum TimerType {
  NO_TTIMER = 0,

  kTotal,
  kSumCyclesTotal,
  kSumCyclesBlock,
  kSumCyclesRun,

  LEVEL_ZERO,

  kExecuteComputationJob,
  kExecuteCopyJob,
  kExecuteParentJob,

  LEVEL_ONE,

  kInvalidateMappings,
  kClearAfterSet,
  kRCRCopy,
  kJobGraph1,
  kJobGraph2,
  kJobGraph3,
  kJobGraph4,
  kExecuteWriteJob,
  kDataExchangerLock,

  LEVEL_TWO,

  kSyncData,
  kAssemblingCache,
  kCoreCommand,
  kCoreTransmission,
  kCoreJobDone,
  kReadAppData,
  kWriteAppData,
  kReadAppDataField,
  kReadAppDataParticle,
  kWriteAppDataField,
  kWriteAppDataParticle,
  kInitializeCalculateDt,

  LEVEL_THREE,

  k1, k2, k3, k4, k5, k6, k7, k8, k9,
  k10, k11, k12, k13, k14, k15, k16, k17, k18, k19, k20,
  kMaxTimer
};

struct TimerRecord {
  TimerRecord() : depth(0), sum(0) {
  }
  struct timespec old_timestamp;
  struct timespec new_timestamp;
  int64_t depth;
  int64_t sum;
};
typedef std::map<std::pair<pid_t, TimerType>, TimerRecord*> TimersMap;
extern TimersMap timers_map;
extern pthread_key_t timer_keys[kMaxTimer];

enum CounterType {
  kNumIteration = 0,
  kCalculateDt,
  kMaxCounter
};
struct CounterRecord {
  CounterRecord() : sum(0) {
  }
  int64_t sum;
};
typedef std::map<std::pair<pid_t, CounterType>, CounterRecord*> CountersMap;
extern CountersMap counters_map;
extern pthread_key_t counter_keys[kMaxCounter];

void InitializeKeys();
void InitializeTimers();

/*
 * Dump all the timer information to file "output", which is opened by the
 * caller.
 */
void PrintTimerSummary(FILE* output = stdout);

/*
 * Return the sum of all timers with the specified type
 */
int64_t ReadTimerTypeSum(TimerType type);

/*
 * Start a timer whose type is "timer_type". Default "depth" value should be
 * used normally.
 */
inline void StartTimer(TimerType timer_type, int depth = 1) {
  if (timer_type > ttimer_level) {
    return;
  }
  assert(depth > 0);
  void* ptr = pthread_getspecific(timer_keys[timer_type]);
  TimerRecord* record = static_cast<TimerRecord*>(ptr);
  assert(record);
#ifdef __MACH__  // OS X does not have clock_gettime, use clock_get_time
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  (record->new_timestamp).tv_sec = mts.tv_sec;
  (record->new_timestamp).tv_nsec = mts.tv_nsec;
#else
  clock_gettime(CLOCK_REALTIME, &(record->new_timestamp));
#endif
  if (record->depth != 0) {
    record->sum +=
        record->depth * (
             (record->new_timestamp.tv_sec - record->old_timestamp.tv_sec) * 1e9
             + record->new_timestamp.tv_nsec - record->old_timestamp.tv_nsec);
  }
  record->old_timestamp = record->new_timestamp;
  record->depth += depth;
}

/*
 * Stop a timer whose type is "timer_type". Default "depth" value should be
 * used normally.
 */
inline void StopTimer(TimerType timer_type, int depth = 1) {
  if (timer_type > ttimer_level) {
    return;
  }
  assert(depth > 0);
  void* ptr = pthread_getspecific(timer_keys[timer_type]);
  TimerRecord* record = static_cast<TimerRecord*>(ptr);
  assert(record);
#ifdef __MACH__  // OS X does not have clock_gettime, use clock_get_time
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  (record->new_timestamp).tv_sec = mts.tv_sec;
  (record->new_timestamp).tv_nsec = mts.tv_nsec;
#else
  clock_gettime(CLOCK_REALTIME, &(record->new_timestamp));
#endif
  record->sum +=
      record->depth * (
           (record->new_timestamp.tv_sec - record->old_timestamp.tv_sec) * 1e9
           + record->new_timestamp.tv_nsec - record->old_timestamp.tv_nsec);
  record->old_timestamp = record->new_timestamp;
  record->depth -= depth;
  assert(record->depth >= 0);
}

inline int64_t ReadTimer(TimerType timer_type) {
  if (timer_type > ttimer_level) {
    return 0;
  }
  void* ptr = pthread_getspecific(timer_keys[timer_type]);
  TimerRecord* record = static_cast<TimerRecord*>(ptr);
  assert(record);
#ifdef __MACH__  // OS X does not have clock_gettime, use clock_get_time
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  (record->new_timestamp).tv_sec = mts.tv_sec;
  (record->new_timestamp).tv_nsec = mts.tv_nsec;
#else
  clock_gettime(CLOCK_REALTIME, &(record->new_timestamp));
#endif
  return record->sum +
      record->depth * (
           (record->new_timestamp.tv_sec - record->old_timestamp.tv_sec) * 1e9
           + record->new_timestamp.tv_nsec - record->old_timestamp.tv_nsec);
}

inline void AddCounter(CounterType counter_type, int count = 1) {
  void* ptr = pthread_getspecific(counter_keys[counter_type]);
  CounterRecord* record = static_cast<CounterRecord*>(ptr);
  assert(record);
  record->sum += count;
}

}  // namespace timer
}  // namespace nimbus


#endif  // NIMBUS_SRC_SHARED_FAST_LOG_H_
