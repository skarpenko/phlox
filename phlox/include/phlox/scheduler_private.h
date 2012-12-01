/*
* Copyright 2007-2010, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_SCHEDULER_PRIVATE_H_
#define _PHLOX_SCHEDULER_PRIVATE_H_

#include <phlox/list.h>
#include <phlox/spinlock.h>
#include <phlox/param.h>
#include <phlox/thread_private.h>


/* Ticks and milliseconds conversion macroses */
#define SCHED_TICKS2MSEC(a)  ( (a) * ((1000L)+(1000L)%HZ) / HZ )
#define SCHED_MSEC2TICKS(a)  ( (a) * HZ / ((1000L)+(1000L)%HZ) )


/* Scheduling ticks types */
#define SCHED_TICK_TYPES_COUNT  8
#define SCHED_TICK_TYPES_MASK  (SCHED_TICK_TYPES_COUNT-1)
#define SCHED_ESTIMATION_TICK  (SCHED_TICK_TYPES_COUNT-1)

/* Quanta reestimation threshold (power of 2) */
#define SCHED_QUANTA_THRESHOLD2  1

/* Priority recalculation factor parameters */
#define SCHED_FACTOR_BASE   (THREAD_DEFAULT_QUANTA)
#define SCHED_FACTOR_A      200
#define SCHED_FACTOR_BTHR    50
#define SCHED_FACTOR_BMAX     8

/* Per priority queue bonus threshold (power of 2) */
#define SCHED_QUEUE_BONUS_THRESHOLD2  4
/* Better thread lookahead depth */
#define SCHED_QUEUE_LOOKAHEAD_DEPTH  10

/* Maximum thread bonus and penalty */
#define SCHED_MAX_THREAD_BONUS     8
#define SCHED_MAX_THREAD_PENALTY   8


/* Redefinition for convenience */
typedef xlist_t sched_queue_t;

/* Per CPU run queue */
typedef struct {
    spinlock_t     lock;                               /* Access lock */
    uint           cpu_num;                            /* CPU number */
    uint           total_count;                        /* Total threads count */
    sched_queue_t  queue[THREAD_NUM_PRIORITY_LEVELS];  /* Priority queues */
} runqueue_t;


/* check for possible constants errors */
#if !SCHED_MSEC2TICKS(THREAD_DEFAULT_QUANTA)
#  error THREAD_DEFAULT_QUANTA has 0 ticks length! Increase HZ or change THREAD_DEFAULT_QUANTA!
#endif

#if !SCHED_MSEC2TICKS(THREAD_MINIMUM_QUANTA)
#  error THREAD_MINIMUM_QUANTA has 0 ticks length! Increase HZ or change THREAD_MINIMUM_QUANTA!
#endif

#if !SCHED_MSEC2TICKS(THREAD_QUANTA_GRANULARITY)
#  error THREAD_QUANTA_GRANULARITY has 0 ticks length! Increase HZ or change THREAD_QUANTA_GRANULARITY!
#endif

#if !SCHED_MSEC2TICKS(SCHED_FACTOR_BASE)
#  error SCHED_FACTOR_BASE has 0 ticks length! Increase HZ or change SCHED_FACTOR_BASE!
#endif

#endif
