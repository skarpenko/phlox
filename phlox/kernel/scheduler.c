/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <sys/debug.h>
#include <phlox/errors.h>
#include <phlox/list.h>
#include <phlox/spinlock.h>
#include <phlox/thread_private.h>
#include <phlox/scheduler.h>


/*** Public routines ***/

/* scheduler init */
status_t scheduler_init(kernel_args_t *kargs)
{
    status_t err;

    /* arch-specific init */
    err = arch_scheduler_init(kargs);
    if(err != NO_ERROR)
        return err;

    return NO_ERROR;
}

/* scheduler per cpu init */
status_t scheduler_init_per_cpu(kernel_args_t *kargs, uint curr_cpu)
{
    status_t err;

    /* arch-specific per cpu init */
    err = arch_scheduler_init_per_cpu(kargs, curr_cpu);
    if(err != NO_ERROR)
        return err;

    return NO_ERROR;
}

/* called from timer tick handler */
bool scheduler_timer(void)
{
    return false;
}

/* reschedules and performs context switch */
void scheduler_resched(void)
{
}
