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
#define CPU1_TSS          0x38
#define CPU2_TSS          0x40
#define CPU3_TSS          0x48
#define CPU4_TSS          0x50
#define CPU5_TSS          0x58
#define CPU6_TSS          0x60
#define CPU7_TSS          0x68
/* and so on... */

#endif
