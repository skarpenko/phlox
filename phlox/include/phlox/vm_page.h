/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_VM_PAGE_H_
#define _PHLOX_VM_PAGE_H_

#include <phlox/ktypes.h>
#include <phlox/kernel.h>
#include <phlox/kargs.h>
#include <phlox/vm_types.h>


/*
 * Pre initialization routine. Called during VM init stage.
 * Computes memory sizes and sets memory statistics structure.
 */
status_t vm_page_preinit(kernel_args_t *kargs);

/*
 * Page module initialization. Called during VM init stage.
 * Initializes page lists.
 */
status_t vm_page_init(kernel_args_t *kargs);

/*
 * Final stage of module initialization. Called during VM init stage.
 * Initializes memory map data.
 */
status_t vm_page_init_final(kernel_args_t *kargs);

/*
 * Mark given physical page as in use, but still not
 * used by VM
*/
status_t vm_page_mark_page_inuse(addr_t page);

/*
 * Mark given range of physical pages as in use, but still not
 * used by VM
*/
status_t vm_page_mark_range_inuse(addr_t start_page, size_t len_pages);

/*
 * Allocate specific page by its physical number and
 * state in VM subsystem
*/
vm_page_t *vm_page_alloc_specific(addr_t page_num, uint page_state);

/*
 * Allocate specific continuous range of pages by physical number of
 * first page in the range.
*/
vm_page_t *vm_page_alloc_specific_range(addr_t first_page_num, addr_t npages, uint page_state);

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
