/*

Note: Idle priority is max penalty size, imho

*/

#ifndef __SCHED_PRIVATE_H__
#define __SCHED_PRIVATE_H__

#include <list.h>
#include <thread_private.h>

#define HZ 250 /* timer tick rate */

#define SCHED_TICK_MSEC (1000 / HZ) /* tick length in msec */
#define SCHED_TICKS2MSEC(a) ( (a) * SCHED_TICK_MSEC )
#define SCHED_MSEC2TICKS(a) ( (a) / SCHED_TICK_MSEC )

#define SCHED_TICK_TYPES_COUNT  8   /* 4 or 8 */
#define SCHED_TICK_TYPES_MASK  (SCHED_TICK_TYPES_COUNT-1)
#define SCHED_ESTIMATION_TICK  (SCHED_TICK_TYPES_COUNT-1)

#define SCHED_QUANTA_THRESHOLD2 1

#define SCHED_FACTOR_BASE   (THREAD_DEFAULT_QUANTA)
#define SCHED_FACTOR_A       200
#define SCHED_FACTOR_BTHR    50
#define SCHED_FACTOR_BMAX     8

#define SCHED_QUEUE_BONUS_THRESHOLD2 4
#define SCHED_QUEUE_LOOKAHEAD_DEPTH 10

#define SCHED_MAX_THREAD_BONUS     8
#define SCHED_MAX_THREAD_PENALTY   8

typedef xlist_t sched_queue_t;

typedef struct {
    spinlock_t lock;
    uint total_count;
    sched_queue_t  queue[THREAD_NUM_PRIORITY_LEVELS];
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
