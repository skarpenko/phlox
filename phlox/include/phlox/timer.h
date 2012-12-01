/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_TIMER_H_
#define _PHLOX_TIMER_H_

#include <phlox/ktypes.h>
#include <phlox/kernel.h>
#include <phlox/kargs.h>
#include <phlox/arch/timer.h>
#include <phlox/platform/timer.h>


/*
 * Timer init routine. Called on system start up.
*/
status_t timer_init(kernel_args_t *kargs);


/*
 * Called on timer tick. Do not call directly.
*/
flags_t timer_tick(void);


#endif
