/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/errors.h>
#include <phlox/process.h>
#include <phlox/thread_private.h>


/* architecture-specific process module init */
status_t arch_process_init(kernel_args_t *kargs)
{
    /* nothing init for now */
    return NO_ERROR;
}

/* architecture-specific process module per CPU init */
status_t arch_process_init_per_cpu(kernel_args_t *kargs, uint curr_cpu)
{
    /* nothing for init */
    return NO_ERROR;
}

/* init arch-specific field of process structure */
status_t arch_init_process_struct(arch_process_t *arch)
{
    return NO_ERROR;
}
