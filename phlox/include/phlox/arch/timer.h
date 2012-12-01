/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_TIMER_H
#define _PHLOX_ARCH_TIMER_H

#include INC_ARCH(phlox/arch,timer.h)


/*
 * Architecture-specific timer initialization.
 * Called when system starts.
*/
status_t arch_timer_init(kernel_args_t *kargs);


#endif
