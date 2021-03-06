/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_SCHEDULER_H_
#define _PHLOX_ARCH_SCHEDULER_H_

#include <phlox/types.h>
#include <phlox/ktypes.h>
#include <phlox/kargs.h>
#include INC_ARCH(phlox/arch,scheduler.h)


/*
 * Architecure-specific scheduler init.
 * Called during threading init!
*/
status_t arch_scheduler_init(kernel_args_t *kargs);

/*
 * Architecture-specific per CPU scheduler init.
 * Called only during threading init!
*/
status_t arch_scheduler_init_per_cpu(kernel_args_t *kargs, uint curr_cpu);

/*
 * Switch execution context from one thread to another.
*/
void arch_sched_context_switch(thread_t *t_from, thread_t *t_to);


#endif
