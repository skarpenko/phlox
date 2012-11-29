/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_VM_H
#define _PHLOX_ARCH_VM_H

#include INC_ARCH(phlox/arch,vm.h)

/*
 * Architecture specific virtual memory initialization
 * Called during system initialization.
*/
status_t arch_vm_init(kernel_args_t *kargs);

#endif
