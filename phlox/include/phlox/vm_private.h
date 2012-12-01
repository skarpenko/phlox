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

/*******************************************************************
 * VM INTERNALS. DO NOT USE!
 *******************************************************************/


/*
 * Address spaces module init
 */
status_t vm_address_spaces_init(kernel_args_t *kargs);

/*
 * Memory objects module init
 */
status_t vm_objects_init(kernel_args_t *kargs);

/*
 * Page fault handler
 */
result_t vm_hard_page_fault(addr_t addr, addr_t fault_addr, bool is_write, bool is_exec, bool is_user);

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
 * Initializes physical pages reference counters
 * for already mapped pages to kernel's address space.
 * Used only during VM subsystem init!
 */
status_t vm_page_init_wire_counters(addr_t vbase, size_t size);

/*
 * Create kernel address space.
 * Called only once during system init stage.
 */
aspace_id vm_create_kernel_aspace(const char* name, addr_t base, size_t size);

/*
 * Create mapping of specified size and put it into memory map of address space.
 * Address space access lock must be acquired before call!
 */
status_t vm_aspace_create_mapping(vm_address_space_t *aspace, size_t size, vm_mapping_t **mapping);

/*
 * Create mapping of specified size at given address and put it into
 * memory map of address space. Memory gap at given address must be free.
 * Address space access lock must be acquired before call!
 */
status_t vm_aspace_create_mapping_exactly(vm_address_space_t *aspace, addr_t base, size_t size, vm_mapping_t **mapping);

/*
 * Delete mapping from address space.
 * Address space access lock must be acquired before call!
 */
void vm_aspace_delete_mapping(vm_address_space_t *aspace, vm_mapping_t *mapping);

/*
 * Get mapping by address that is covered by in memory map.
 * Address space access lock must be acquired before call!
 */
status_t vm_aspace_get_mapping(vm_address_space_t *aspace, addr_t vaddr, vm_mapping_t **mapping);

/*
 * Create new universal page at given offset within memory object.
 * Object access lock must be acquired before call!
 */
status_t vm_object_add_upage(vm_object_t *object, addr_t offset, vm_upage_t **upage);

/*
 * Get or create universal page at given offset within memory object.
 * Object access lock must be acquired before call!
 */
status_t vm_object_get_or_add_upage(vm_object_t *object, addr_t offset, vm_upage_t **upage);

/*
 * Get universal page at given offset within memory object.
 * Object access lock must be acquired before call!
 */
status_t vm_object_get_upage(vm_object_t *object, addr_t offset, vm_upage_t **upage);

/*
 * Put mapping wired with object into its internal list of mappings.
 * Object access lock must be acquired before call!
 */
void vm_object_put_mapping(vm_object_t *object, vm_mapping_t *mapping);

/*
 * Remove mapping wired with object from its internal list of mappings.
 * Object access lock must be acquired before call!
 */
void vm_object_remove_mapping(vm_object_t *object, vm_mapping_t *mapping);


#endif
