/*
* Copyright 2007-2012, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <arch/i386/fpu_data.h>
#include <phlox/errors.h>
#include <phlox/arch/i386/segments.h>
#include <phlox/processor.h>
#include <phlox/thread.h>
#include <phlox/thread_private.h>


/* initial fpu state for newborn threads */
static fpu_state initial_fpu_state _ALIGNED(16);


/* architecture-dependend threading initialization */
status_t arch_threading_init(kernel_args_t *kargs)
{
    /* prepare global valid FPU state */
    asm volatile (
     "   clts     ;"  /* clear Task Switched flag to avoid exceptions */
     "   fninit   ;"  /* init FPU */
     "   fnclex   ;"  /* clear exceptions flags */
    );
    /* save prepared fpu state */
    i386_fpu_context_save(&initial_fpu_state);

    /* return success */
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

    /* copy initial fpu state */
    memcpy(thread->arch.fpu_state, &initial_fpu_state, sizeof(fpu_state));

    /* done */
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

/* transfers control to user space for newly created user thread */
void arch_thread_enter_uspace(thread_t *thread)
{
    /* TODO: Locate and map return stub object here ?*/

    /* reinit FPU to ensure it is in good state */
    asm("fninit");

    /* touch user stack to ensure its top page mapped */
    TOUCH_ADDR(thread->ustack_top - 4);

    /* pass control to user code */
    i386_enter_uspace(thread->entry, thread->data, thread->ustack_top, 0 /* ret addr */);
}

/* called after just created thread gets control */
status_t arch_thread_first_start_init(thread_t *thread)
{
    /* nothing todo here at this time */

    return NO_ERROR;
}
