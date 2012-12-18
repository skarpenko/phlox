/*
* Copyright 2007-2012, Stepan V.Karpenko. All rights reserved.
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

/*
 * Initialize architecture-dependend parts of thread structure.
*/
status_t arch_thread_init_struct(thread_t *thread);

/*
 * Initialize kernel stack of newly created thread.
 * stub_func will be called first when new thread will be
 * scheduled for execution.
*/
status_t arch_thread_init_kstack(thread_t *thread, int (*stub_func)(void));

/*
 * Transfer control to user space code.
 * Called once for new user thread after it scheduled
 * for execution at first time. This function never returns.
*/
void arch_thread_enter_uspace(thread_t *thread);

/*
 * Init new thread after it gets control at first time
*/
status_t arch_thread_first_start_init(thread_t *thread);


#endif
