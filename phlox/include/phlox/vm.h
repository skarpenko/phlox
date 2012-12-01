/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_VM_H_
#define _PHLOX_VM_H_

#include <phlox/ktypes.h>
#include <phlox/kernel.h>
#include <phlox/kargs.h>
#include <phlox/arch/vm.h>
#include <phlox/platform/vm.h>

/* Memory protection attributes */
#define VM_PROT_READ     0x01  /* Read permission            */
#define VM_PROT_WRITE    0x02  /* Write permission           */
#define VM_PROT_EXECUTE  0x04  /* Execute permission         */
#define VM_PROT_KERNEL   0x08  /* Kernel's memory            */
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


/* Returns available physical memory
 * size installed in the system.
 */
size_t vm_phys_mem_size(void);


/* Global variable with Virtual Memory State
 * Modified only by VM internals, use only
 * for information.
 */
extern vm_stat_t VM_State;

#endif
