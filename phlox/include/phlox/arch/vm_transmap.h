/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_VM_TRANSMAP_H
#define _PHLOX_ARCH_VM_TRANSMAP_H

#include INC_ARCH(phlox/arch,vm_transmap.h)

/*
 * Map a page directly without using any of Virtual Memory Manager objects.
 * This routine used only during system start up. Do not use after.
 */
int vm_transmap_quick_map(kernel_args_t *kargs, addr_t va, addr_t pa, uint32 attributes);

#endif
