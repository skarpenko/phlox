/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_PLATFORM_VM_H
#define _PHLOX_PLATFORM_VM_H

#include INC_PLATF(phlox/platform,vm.h)

/*
 * Platform-specific virtual memory initialization.
 * Called during system initialization.
*/
status_t platform_vm_init(kernel_args_t *kargs);

/*
 * Final stage of platform-specific VM initialization.
 * Called on final stage of VM initialization.
*/
status_t platform_vm_init_final(kernel_args_t *kargs);

#endif
