/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/types.h>
#include <phlox/errors.h>
#include <phlox/kernel.h>
#include <phlox/processor.h>
#include <phlox/arch/i386/exceptions.h>


/* called on first use of fpu by thread */
status_t i386_device_not_available(void)
{
   clts(); /* let thread use fpu */

   return NO_ERROR;
}
