/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_VM_H_
#define _PHLOX_VM_H_

#include <phlox/ktypes.h>
#include <phlox/kernel.h>
#include <phlox/kargs.h>
#include <phlox/arch/vm.h>
#include <phlox/platform/vm.h>
#include <phlox/vm_types.h>


/* Memory protection attributes */
#define VM_PROT_READ     0x01  /* Read permission            */
#define VM_PROT_WRITE    0x02  /* Write permission           */
#define VM_PROT_EXECUTE  0x04  /* Execute permission         */
#define VM_PROT_KERNEL   0x08  /* Kernels memory             */
#define VM_PROT_MASK     0x0F  /* Protection attributes mask */
#define VM_PROT_KERNEL_ALL      (VM_PROT_KERNEL | VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE)
#define VM_PROT_KERNEL_DEFAULT  (VM_PROT_KERNEL | VM_PROT_READ | VM_PROT_WRITE)
#define VM_PROT_KERNEL_NONE     (VM_PROT_KERNEL)
#define VM_PROT_USER_ALL        (VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE)
#define VM_PROT_USER_DEFAULT    (VM_PROT_READ | VM_PROT_WRITE)
#define VM_PROT_USER_NONE       (0)

/* Page state flags */
#define VM_FLAG_PAGE_PRESENT   0x10  /* Page present  */
#define VM_FLAG_PAGE_MODIFIED  0x20  /* Page modified */
#define VM_FLAG_PAGE_ACCESSED  0x40  /* Page accessed */
#define VM_FLAG_PAGE_MASK      0x70  /* Mask          */

/* Reserved ID values */
#define VM_INVALID_OBJECTID  ((object_id)0)  /* Invalid object ID */
#define VM_INVALID_ASPACEID  ((aspace_id)0)  /* Invalid address space ID */


/*
 * State of the Virtual Memory.
 * Used for statistics and informational purposes.
 */
typedef struct {
    /*** Amount of RAM in the system ***/
    uint  physical_page_size;
    uint  total_physical_pages;

    /*** Page lists ***/
    uint  active_pages;
    uint  inactive_pages;
    uint  wired_pages;
    uint  busy_pages;
    uint  clear_pages;
    uint  free_pages;
    uint  unused_pages;
} vm_stat_t;


/*
 * This routine called during system init stage
 * for virtual memory initialization.
*/
status_t vm_init(kernel_args_t *kargs);

/*
 * This routine called after threading init stage to
 * continue VM initialization.
*/
status_t vm_init_post_threading(kernel_args_t *kargs);

/*
 * This routine called after semaphores init stage to
 * continue VM initialization.
*/
status_t vm_init_post_sema(kernel_args_t *kargs);

/*
 * Returns available physical memory
 * size installed in the system.
*/
size_t vm_phys_mem_size(void);

/*
 * Creates address space with given name, base address and size.
 * Returns id of newly created address space or VM_INVALID_ASPACEID
 * on error.
*/
aspace_id vm_create_aspace(const char* name, addr_t base, size_t size);

/*
 * Deletes address space by its id.
 * Returns ERR_VM_INVALID_ASPACE or NO_ERROR.
*/
status_t vm_delete_aspace(aspace_id aid);

/*
 * Returns kernel address space
*/
vm_address_space_t *vm_get_kernel_aspace(void);

/*
 * Returns kernel address space id
*/
aspace_id vm_get_kernel_aspace_id(void);

/*
 * Returns current user address space.
 * If called inside kernel thread returns NULL.
*/
vm_address_space_t *vm_get_current_user_aspace(void);

/*
 * Returns current user address space id.
 * If called inside kernel thread returns VM_INVALID_ASPACEID.
*/
aspace_id vm_get_current_user_aspace_id(void);

/*
 * Get address space by its id.
 * Returns address space or NULL if no address space
 * found with given id.
*/
vm_address_space_t* vm_get_aspace_by_id(aspace_id aid);

/*
 * Put previously taken address space
*/
void vm_put_aspace(vm_address_space_t *aspace);

/*
 * Returns address space id by its name.
 * If no address space found returns VM_INVALID_ASPACEID.
*/
aspace_id vm_find_aspace_by_name(const char *name);

/*
 * Increment address space references count.
 * Returns address space or NULL if address space is not
 * available anymore.
*/
vm_address_space_t *vm_inc_aspace_refcnt(vm_address_space_t *aspace);

/*
 * Creates memory object with given name, size and protection.
 * Returns id of newly created object or VM_INVALID_OBJECTID
 * on error.
*/
object_id vm_create_object(const char *name, size_t size, uint protection);

/*
 * Creates memory object with given name, size and protection and
 * assigns chunk of physical pages to it starting from phys_addr.
 * Returns id of newly created object or VM_INVALID_OBJECTID
 * on error.
*/
object_id vm_create_physmem_object(const char *name, addr_t phys_addr, size_t size, uint protection);

/* Creates memory object with given properties and assigns physical
 * pages to it that is queried from virtual addresses within given
 * address space.
 * Returns id of newly created object or VM_INVALID_OBJECTID
 * on error.
 * This routine used on system start up. Do not use later.
*/
object_id vm_create_virtmem_object(const char *name, aspace_id aid, addr_t virt_addr, size_t size, uint protection);

/*
 * Deletes memory object by its id.
 * Returns ERR_VM_INVALID_OBJECT or NO_ERROR.
*/
status_t vm_delete_object(object_id oid);

/*
 * Get object by its id.
 * Returns object or NULL if no object
 * found with given id.
*/
vm_object_t *vm_get_object_by_id(object_id oid);

/*
 * Put previously taken object
*/
void vm_put_object(vm_object_t *object);

/*
 * Returns object id by its name.
 * If no object found returns VM_INVALID_OBJECTID.
*/
object_id vm_find_object_by_name(const char *name);

/*
 * Creates memory hole of specified size at given address.
*/
status_t vm_create_memory_hole(aspace_id aid, addr_t base, size_t size);

/*
 * Deletes memory hole at given address.
*/
status_t vm_delete_memory_hole(aspace_id aid, addr_t vaddr);

/*
 * Map object into address space.
 *  INPUTS:
 *   protection - memory protection flags;
 *   *vaddr - recieves address where object mapped.
*/
status_t vm_map_object(aspace_id aid, object_id oid, uint protection, addr_t *vaddr);

/*
 * Map object into address space at address aligned by specified
 * pages count.
 *  INPUTS:
 *   protection - memory protection flags;
 *   npg_align - number of pages to align base address,
 *               0 and 1 values acts the same way;
 *   *vaddr - recieves address where object mapped.
*/
status_t vm_map_object_aligned(aspace_id aid, object_id oid, uint protection, uint npg_align, addr_t *vaddr);

/*
 * Map object exactly at specified address of address space.
 *  INPUTS:
 *   protection - memory protection flags;
 *   vaddr - address where object must be mapped.
*/
status_t vm_map_object_exactly(aspace_id aid, object_id oid, uint protection, addr_t vaddr);

/*
 * Unmap previously mapped object at specified address
 * of address space.
*/
status_t vm_unmap_object(aspace_id aid, addr_t vaddr);

/*
 * Simulate page fault for given virtual address range.
 * Calls soft page fault handler for each page in range,
 * so already mapped pages will be remapped again.
 * Use with care.
*/
status_t vm_simulate_pf(addr_t start, addr_t end);

/*
 * Query physical address and flags for given virtual address.
 *  INPUTS:
 *   aspace    - address space;
 *   vaddr     - virtual address;
 *   out_paddr - output physical address;
 *   out_flags - output flags (may be NULL).
*/
status_t vm_query_paddr(vm_address_space_t *aspace, addr_t vaddr, addr_t *out_paddr, uint *out_flags);


/*
 * Global variable with Virtual Memory State
 * Modified only by VM internals, use only
 * for information.
 */
extern vm_stat_t VM_State;


#endif
