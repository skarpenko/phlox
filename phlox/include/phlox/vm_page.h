/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_VM_PAGE_H_
#define _PHLOX_VM_PAGE_H_

#include <phlox/ktypes.h>
#include <phlox/kernel.h>
#include <phlox/kargs.h>


/*** The following routines used during system initialization do not use them ***/

/*
 * Page module initialization
 */
uint32 vm_page_init(kernel_args_t *kargs);

/* Allocate virtual space of given size from kernel args structure. */
addr_t vm_alloc_vspace_from_kargs(kernel_args_t *kargs, uint32 size);
/* Allocate free physical page from kernel args structure. */
addr_t vm_alloc_phpage_from_kargs(kernel_args_t *kargs);
/*
 * Allocate memory block of given size form kernel args structure.
 * Attributes parameter specifies access rights to allocated block.
*/
addr_t vm_alloc_from_kargs(kernel_args_t *kargs, uint32 size, uint32 attributes);

#endif
