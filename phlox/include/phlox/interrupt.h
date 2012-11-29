/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_INTERRUPT_H_
#define _PHLOX_INTERRUPT_H_

#include <phlox/types.h>
#include <phlox/kargs.h>
#include <phlox/platform/irq.h>
#include <phlox/arch/interrupt.h>

/*
 * Interrupt handling initialization routine.
 * Called during kernel startup.
 */
uint32 interrupt_init(kernel_args_t *kargs);

#endif
