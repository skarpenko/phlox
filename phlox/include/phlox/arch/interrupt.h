/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_INTERRUPT_H_
#define _PHLOX_ARCH_INTERRUPT_H_

#include INC_ARCH(phlox/arch,interrupt.h)

/*
 * Architecture-specific interrupt handling
 * initialization routine.
 * Called during kernel startup.
 */
uint32 arch_interrupt_init(kernel_args_t *kargs);

#endif
