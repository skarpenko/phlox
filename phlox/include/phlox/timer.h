/*
* Copyright 2007-2010, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_TIMER_H_
#define _PHLOX_TIMER_H_

#include <phlox/ktypes.h>
#include <phlox/kernel.h>
#include <phlox/kargs.h>
#include <phlox/param.h>
#include <phlox/arch/timer.h>
#include <phlox/platform/timer.h>
#include <phlox/thread_types.h>


/* Parameters used in time convertion */
#define TIMER_MSEC_PER_SEC  (1000L)
#define TIMER_USEC_PER_SEC  (1000000L)
#define TIMER_NSEC_PER_SEC  (1000000000L)

/* Timer ticks to time and time to timer ticks conversion */
#define TIMER_TICKS_TO_TIME(tick,u)  ( (tick) * ((u)+(u)%HZ) / HZ )
#define TIMER_TIME_TO_TICKS(time,u)  ( (time) * HZ / ((u)+(u)%HZ) )
#define TIMER_TICKS_TO_MSEC(a)       TIMER_TICKS_TO_TIME(a, TIMER_MSEC_PER_SEC)
#define TIMER_MSEC_TO_TICKS(a)       TIMER_TIME_TO_TICKS(a, TIMER_MSEC_PER_SEC)


/*
 * Timer init routine. Called on system start up.
*/
status_t timer_init(kernel_args_t *kargs);


/*
 * Called on timer tick. Do not call directly.
*/
flags_t timer_tick(void);


/*
 * Returns count of timer ticks counted since system booted up.
*/
bigtime_t timer_get_ticks(void);

/*
 * Returns count of milliseconds since system booted up.
*/
bigtime_t timer_get_time(void);

/*
 * Suspends thread for a given amount of timer ticks.
 * Thread must be locked before call and be in RUNNING
 * or READY state. After reschedule thread will be in
 * a SLEEPING state.
*/
status_t timer_lull_thread(thread_t *thread, uint ticks);


#endif
