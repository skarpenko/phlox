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
 * This routine called during system init stage
 * for virtual memory initialization.
*/
void init_vm(kernel_args_t *kargs);

#endif
