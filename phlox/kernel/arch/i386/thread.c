/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/errors.h>
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

