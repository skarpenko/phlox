/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/errors.h>
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

