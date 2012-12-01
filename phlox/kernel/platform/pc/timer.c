/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/errors.h>
#include <phlox/types.h>
#include <phlox/interrupt.h>
#include <phlox/timer.h>


/* timer interrupt handler */
static flags_t timer_int_handler(void *data)
{
    return timer_tick();
}


/* platform-specific timer init */
status_t platform_timer_init(kernel_args_t *kargs)
{
    status_t err;

    /* set timer interrupt handler */
    err = set_hw_interrupt_handler(0, &timer_int_handler, "timer_int_handler", NULL);

    return err;
}
