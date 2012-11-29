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
    uint ppn;
    /* references count to page */
    uint ref_count;
    /* page type */
    uint type  : 2;
    /* page state */
    uint state : 4;
} vm_page_t;

/* Page types */
enum {
    VM_PAGE_TYPE_PHYS = 0 /* Physical page */
};

/* Page states */
enum {
    VM_PAGE_STATE_ACTIVE = 0,  /* Active page */
    VM_PAGE_STATE_FREE,        /* Free page */
    VM_PAGE_STATE_CLEAR,       /* Clear page */
    VM_PAGE_STATE_WIRED,       /* Wired page */
    VM_PAGE_STATE_BUSY,        /* Busy page */
    VM_PAGE_STATE_UNUSED       /* Unused or reserved page */
};


/*** The following routines used during system initialization do not use them ***/

/*
 * Pre initialization routine.
 * Computes memory sizes and sets memory statistics structure.
 */
status_t vm_page_preinit(kernel_args_t *kargs);

/*
 * Page module initialization.
 * Initializes page lists.
 */
status_t vm_page_init(kernel_args_t *kargs);

/*
 * Allocate free physical page from kernel args structure.
 * Used on system init stage.
 */
addr_t vm_alloc_ppage_from_kargs(kernel_args_t *kargs);


/*
 * Mark given physical page as in use, but still not
 * used by VM
 *
*/
status_t vm_page_mark_page_inuse(addr_t page);

/*
 * Mark given range of physical pages as in use, but still not
 * used by VM
 *
*/
status_t vm_page_mark_range_inuse(addr_t start_page, size_t len_pages);

/*
 * Allocate specific page by its physical number and
 * state in VM subsystem
*/
vm_page_t *vm_page_alloc_specific(addr_t page_num, uint page_state);

/*
 * Allocate page by state in VM subsystem
*/
vm_page_t *vm_page_alloc(uint page_state);

/*
 * Allocate continuous range of pages by state in VM subsystem
*/
vm_page_t *vm_page_alloc_range(uint page_state, addr_t npages);

/*
 * Look up a page
*/
vm_page_t *vm_page_lookup(addr_t page_num);

/*
 * Set page state in VM subsystem
*/
status_t vm_page_set_state(vm_page_t *page, uint page_state);

/*
 * Returns total pages count
*/
size_t vm_page_pages_count(void);

/*
 * Returns free pages count
*/
size_t vm_page_free_pages_count(void);

#endif
