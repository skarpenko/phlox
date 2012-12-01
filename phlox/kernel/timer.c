/*
* Copyright 2007-2010, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/errors.h>
#include <phlox/processor.h>
#include <phlox/interrupt.h>
#include <phlox/scheduler.h>
#include <phlox/timer.h>


/* Timer ticks counter */
static volatile bigtime_t timer_ticks = 0;


/* system timer initialization */
status_t timer_init(kernel_args_t *kargs)
{
    status_t err;

    /* call architecture-specific init routine */
    err = arch_timer_init(kargs);
    if(err != NO_ERROR)
        return err;

    /* call platform-specific init routine */
    err = platform_timer_init(kargs);
    if(err != NO_ERROR)
        return err;

    return NO_ERROR;
}

/* timer tick event handler */
flags_t timer_tick(void)
{
    flags_t ret = INT_FLAGS_NOFLAGS;

    /* count tick */
    ++timer_ticks;

    /* call scheduler */
    if(scheduler_timer())
        ret |= INT_FLAGS_RESCHED;

    return ret;
}

/* return ticks count */
bigtime_t timer_get_ticks(void)
{
    unsigned long irqs_state;
    bigtime_t ticks_cpy;

    /* disable interrupts and copy current ticks count to local var */
    local_irqs_save_and_disable(irqs_state);
    ticks_cpy = timer_ticks;
    /* restore interrupts and return ticks count to caller */
    local_irqs_restore(irqs_state);
    return ticks_cpy;
}

/* return milliseconds count */
bigtime_t timer_get_time(void)
{
    bigtime_t ticks = timer_get_ticks();
    return TIMER_TICKS_TO_MSEC(ticks);
}
