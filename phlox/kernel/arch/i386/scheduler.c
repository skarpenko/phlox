/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/errors.h>
#include <phlox/processor.h>
#include <phlox/scheduler.h>
#include <phlox/thread_private.h>


/* architecure-specific scheduler init */
status_t arch_scheduler_init(kernel_args_t *kargs)
{
   /* nothing to init */
   return NO_ERROR;
}

/* architecture-specific per CPU scheduler init */
status_t arch_scheduler_init_per_cpu(kernel_args_t *kargs, uint curr_cpu)
{
   /* nothing to init */
   return NO_ERROR;
}

/* switch context */
void arch_scheduler_context_switch(thread_t *t_from, thread_t *t_to)
{
    addr_t from_pgdir = (addr_t)t_from->process->aspace->tmap.arch.pgdir_phys;
    addr_t to_pgdir = (addr_t)t_to->process->aspace->tmap.arch.pgdir_phys;

    /* switch page directory if switching to
     * new address space
     */
    if(from_pgdir != to_pgdir)
        i386_pgdir_switch(to_pgdir);

    /* TODO: disable FPU for its usage monitoring */

    /* set new kernel stack for next task switch */
    i386_set_kstack(t_to->kstack_top);

    /* switch to new context */
    i386_context_switch(&t_from->arch, &t_to->arch);
}
