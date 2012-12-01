/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_SCHEDULER_H_
#define _PHLOX_SCHEDULER_H_

#include <phlox/types.h>
#include <phlox/ktypes.h>
#include <phlox/kargs.h>
#include <phlox/thread_types.h>
#include <phlox/arch/scheduler.h>


/*
 * Called only from timer interrupt handler.
 * Returns true if reschedule needed, false if not.
*/
bool scheduler_timer(void);

/*
 * Performs rescheduling and context switch.
*/
void sched_reschedule(void);

/*
 * Capture cpu by current thread. Thread cannot be rescheduled.
 * Returns current value of preemt_count for thread.
*/
int sched_capture_cpu(void);

/*
 * Release cpu captured by current thread.
 * Returns current value of preemt_count for thread.
*/
int sched_release_cpu(void);


#endif
