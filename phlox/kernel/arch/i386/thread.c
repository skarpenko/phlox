/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <phlox/errors.h>
#include <phlox/arch/i386/segments.h>
#include <phlox/thread.h>
#include <phlox/thread_private.h>


/* architecture-dependend threading initialization */
status_t arch_threading_init(kernel_args_t *kargs)
{
    /* nothing to init for now */
    return NO_ERROR;
}

/* per cpu arch-dependend init */
status_t arch_threading_init_per_cpu(kernel_args_t *kargs, uint curr_cpu)
{
    /* nothing to init for now */
    return NO_ERROR;
}

/* init arch-dependend thread structure parts */
status_t arch_thread_init_struct(thread_t *thread)
{
    /* init with zeroes */
    memset(&thread->arch, 0, sizeof(arch_thread_t));

    return NO_ERROR;
}

/* init kernel-side stack for newly created thread */
status_t arch_thread_init_kstack(thread_t *thread, int (*stub_func)(void))
{
    uint32 *kstack_top = (uint32 *)thread->kstack_top;
    int i;

    /* set return address to the specified stub function */
    kstack_top--;
    *kstack_top = (addr_t)stub_func;

    /* simulate initial pushad instruction */
    for(i = 0; i < 8; i++) {
        kstack_top--;
        *kstack_top = 0; /* pushed registers set to 0 */
    }

    /* set stack state */
    thread->arch.current_stack.esp = (uint32)kstack_top;
    thread->arch.current_stack.ss  = KERNEL_DATA_SEG;

    return NO_ERROR;
}
