/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_I386_SEGMENTS_H_
#define _PHLOX_ARCH_I386_SEGMENTS_H_

/*
 *  GDT selectors
 */

/* Kernel segments */
#define KERNEL_CODE_SEG   0x8
#define KERNEL_DATA_SEG   0x10

/* User segments */
#define USER_CODE_SEG     0x1b
#define USER_DATA_SEG     0x23

/* Double fault TSS */
#define DOUBLE_FAULT_TSS  0x28

/* Per CPU TSS */
#define CPU0_TSS          0x30

#endif
