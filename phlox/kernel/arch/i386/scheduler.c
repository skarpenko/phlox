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
void arch_sched_context_switch(thread_t *t_from, thread_t *t_to)
{
    addr_t from_pgdir = (addr_t)t_from->process->aspace->tmap.arch.pgdir_phys;
    addr_t to_pgdir = (addr_t)t_to->process->aspace->tmap.arch.pgdir_phys;

    /* exit if try switch to ourself */
    if(t_from == t_to)
        return;

    /* switch page directory if switching to
     * new address space
     */
    if(from_pgdir != to_pgdir)
        i386_pgdir_switch(to_pgdir);

    /* save / load debug registers */
    {
        uint32 dr7 = read_dr7();

        /* save current debug state if hardware debug used */
        if(dr7 != 0)
            i386_dbg_regs_save(t_from->arch.debug_regs);

        /* load new debug state if hardware debug used in
         * next thread or turn off debugging facilities.
         */
        if(t_to->arch.debug_regs[0] != 0)
            i386_dbg_regs_load(t_to->arch.debug_regs);
        else
            i386_dbg_regs_clear();
    }

    /* save fpu context if needed */
    if(t_from->arch.fpu_used) {
        i386_fpu_context_save((fpu_state *)t_from->arch.fpu_state);
        t_from->arch.fpu_used = false; /* clear flag */
    }

    /* set task switched bit.
     * due to fpu monitoring, "device not available"
     * exception occurs if thread tries to use fpu.
     * so... after this we know that fpu context must to
     * be stored for this thread.
     */
    stts();

    /* set new kernel stack for next task switch */
    i386_set_kstack(t_to->kstack_top);

    /* switch to new context */
    i386_context_switch(&t_from->arch, &t_to->arch);
}
