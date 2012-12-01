/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/types.h>
#include <phlox/errors.h>
#include <phlox/kernel.h>
#include <phlox/processor.h>
#include <phlox/thread.h>
#include <phlox/arch/i386/exceptions.h>


/* called on first use of fpu by thread */
status_t i386_device_not_available(void)
{
   thread_t *thread = thread_get_current_thread();

   /* let thread use fpu */
   clts(); /* clear TS flag */

   /* so... thread tries to use fpu, load its context into fpu. */
   i386_fpu_context_load((fpu_state *)thread->arch.fpu_state);

   /* set flag that thread used fpu */
   thread->arch.fpu_used = true;

   return NO_ERROR;
}
