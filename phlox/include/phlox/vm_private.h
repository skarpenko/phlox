/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_VM_PRIVATE_H_
#define _PHLOX_VM_PRIVATE_H_

#include <phlox/ktypes.h>
#include <phlox/kernel.h>
#include <phlox/kargs.h>
#include <phlox/vm_types.h>


/*
 * Allocate free physical page from kernel args structure.
 * Used on system init stage.
 */
addr_t vm_alloc_ppage_from_kargs(kernel_args_t *kargs);

/*
 * Allocate virtual space of given size from kernel args structure.
 * Used on system init, do not use after!
 */
addr_t vm_alloc_vspace_from_kargs(kernel_args_t *kargs, size_t size);

/*
 * Allocate memory block of given size form kernel args structure.
 * Protection parameter specifies access rights to allocated block.
 * Used on system init, do not use after!
 */
addr_t vm_alloc_from_kargs(kernel_args_t *kargs, size_t size, uint protection);

/*
 * Create kernel address space.
 * Called only once during system init stage.
 */
aspace_id vm_create_kernel_aspace(const char* name, addr_t base, size_t size);


#endif
