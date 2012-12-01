/*
* Copyright 2007-2011, Stepan V.Karpenko. All rights reserved.
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


/* Reserved ID */
#define INVALID_TIMEOUTID  ((timeout_id)0)  /* Invalid timeout ID */

/* Parameters used in time convertion */
#define TIMER_MSEC_PER_SEC  (1000L)
#define TIMER_USEC_PER_SEC  (1000000L)
#define TIMER_NSEC_PER_SEC  (1000000000L)

/* Timer ticks to time and time to timer ticks conversion */
#define TIMER_TICKS_TO_TIME(tick,u)  ( (tick) * ((u)+(u)%HZ) / HZ )
#define TIMER_TIME_TO_TICKS(time,u)  ( (time) * HZ / ((u)+(u)%HZ) )
#define TIMER_TICKS_TO_MSEC(a)       TIMER_TICKS_TO_TIME(a, TIMER_MSEC_PER_SEC)
#define TIMER_MSEC_TO_TICKS(a)       TIMER_TIME_TO_TICKS(a, TIMER_MSEC_PER_SEC)


/* timeout routine type definition */
typedef void (*timeout_routine_t)(timeout_id,void*);


/*
 * Timer init routine. Called on system start up.
*/
status_t timer_init(kernel_args_t *kargs);


/*
 * Timer module init routine. Called on system start up,
 * right after threading subsystem inited.
*/
status_t timer_init_after_threading(kernel_args_t *kargs);


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

/*
 * Schedule timeout call after given count of timer ticks.
 *
 * Params:
 *  routine - routine to be called;
 *  data    - user data to be passed into routine;
 *  ticks   - timer ticks count.
 */
timeout_id timer_timeout_sched(timeout_routine_t routine, void *data, uint ticks);

/*
 * Cancel previously scheduled timeout call.
 */
void timer_timeout_cancel(timeout_id tid);

/*
 * Cancel previously scheduled timeout call.
 * This version is also ensures that the call is not
 * currently running.
 */
void timer_timeout_cancel_sync(timeout_id tid);


#endif
