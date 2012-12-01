/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_THREAD_H_
#define _PHLOX_ARCH_THREAD_H_

#include <phlox/types.h>
#include <phlox/ktypes.h>
#include <phlox/kargs.h>
#include INC_ARCH(phlox/arch,thread.h)


/*
 * Architecture-dependend threading initialization steps.
 * Called during threading init stage.
*/
status_t arch_threading_init(kernel_args_t *kargs);

/*
 * Architecture-dependend threading per CPU initialization.
 * Called during threading init stage.
*/
status_t arch_threading_init_per_cpu(kernel_args_t *kargs, uint curr_cpu);


#endif
