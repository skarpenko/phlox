/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_PLATFORM_TIMER_H
#define _PHLOX_PLATFORM_TIMER_H

#include INC_PLATF(phlox/platform,timer.h)


/*
 * Platform-specific timer initialization.
 * Called when system starts.
*/
status_t platform_timer_init(kernel_args_t *kargs);


#endif
