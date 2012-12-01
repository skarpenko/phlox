/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/errors.h>
#include <phlox/interrupt.h>
#include <phlox/timer.h>


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
    return INT_FLAGS_NOFLAGS;
}
