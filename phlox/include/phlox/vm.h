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

/* Memory access attributes */
#define VM_LOCK_RO      0x0  /* Read-only       */
#define VM_LOCK_RW      0x1  /* Read and write  */
#define VM_LOCK_USER    0x0  /* User's memory   */
#define VM_LOCK_KERNEL  0x2  /* Kernel's memory */
#define VM_LOCK_MASK    0x3  /* Mask */

/*
 * State of the Virtual Memory.
 * Used for statistics and informational purposes.
 */
typedef struct {
    /*** Amount of RAM in the system ***/
    uint32  physical_page_size;
    uint32  total_physical_pages;

    /*** Page lists ***/
    uint32  active_pages;
    uint32  free_pages;
    uint32  unused_pages;
} vm_stat_t;


/*
 * This routine called during system init stage
 * for virtual memory initialization.
*/
uint32 vm_init(kernel_args_t *kargs);


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
