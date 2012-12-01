/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_I386_EXCEPTIONS_H_
#define _PHLOX_ARCH_I386_EXCEPTIONS_H_


/*
 * Device Not Available exception handler.
 * This exception occurs when thread tries
 * to use fpu at first time after context
 * switch.
 */
status_t i386_device_not_available(void);


#endif
