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

/* Memory access attributes */
#define VM_LOCK_RO      0x0  /* Read-only       */
#define VM_LOCK_RW      0x1  /* Read and write  */
#define VM_LOCK_NOEX    0x0  /* Non-executable  */
#define VM_LOCK_EX      0x2  /* Executable      */
#define VM_LOCK_USER    0x0  /* User's memory   */
#define VM_LOCK_KERNEL  0x4  /* Kernel's memory */
#define VM_LOCK_MASK    0x7  /* Mask            */
/* Page state flags */
#define VM_FLAG_PAGE_PRESENT   0x08  /* Page present  */
#define VM_FLAG_PAGE_MODIFIED  0x10  /* Page modified */
#define VM_FLAG_PAGE_ACCESSED  0x20  /* Page accessed */
#define VM_FLAG_PAGE_MASK      0x38  /* Mask          */


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
