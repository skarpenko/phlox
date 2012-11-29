/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/interrupt.h>

/* init interrupt handling */
uint32 interrupt_init(kernel_args_t *kargs)
{
    /* call architecture specific init */
    arch_interrupt_init(kargs);

    return 0;
}

/* handle hardware interrupt */
uint32 handle_hw_interrupt(uint32 hw_vector)
{
    return 0;
}

