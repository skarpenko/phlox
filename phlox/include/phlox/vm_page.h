/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_VM_PAGE_H_
#define _PHLOX_VM_PAGE_H_

#include <phlox/ktypes.h>
#include <phlox/kernel.h>
#include <phlox/kargs.h>
#include <phlox/list.h>

/* Page of memory */
typedef struct {
    /* pages list node */
    list_elem_t list_node;
    /* physical number of the page */
    uint32 ppn;
    /* references count to page */
    uint32 ref_count;
    /* page type */
    uint32 type  : 2;
    /* page state */
    uint32 state : 4;
} vm_page_t;

/* Page types */
enum {
    VM_PAGE_TYPE_PHYS = 0 /* Physical page */
};

/* Page states */
enum {
    VM_PAGE_STATE_ACTIVE = 0,  /* Active page */
    VM_PAGE_STATE_FREE,        /* Free page */
    VM_PAGE_STATE_UNUSED       /* Unused or reserved page */
};


/*** The following routines used during system initialization do not use them ***/

/*
 * Pre initialization routine.
 * Computes memory sizes and sets memory statistics structure.
 */
uint32 vm_page_preinit(kernel_args_t *kargs);

/*
 * Page module initialization.
 * Initializes page lists.
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
