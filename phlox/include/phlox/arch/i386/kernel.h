/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_I386_KERNEL_H_
#define _PHLOX_ARCH_I386_KERNEL_H_

/*** Kernel Memory Layout ***/

/* Kernel base address */
#define KERNEL_BASE 0xC0000000

/* Kernel space size */
#define KERNEL_SIZE 0x40000000

/* Kernel top address */
#define KERNEL_TOP (KERNEL_BASE + KERNEL_SIZE - 1)

/* Kernel address mask */
#define KERNEL_ADDR_MASK 0xC0000000

/* Kernel stack size (in pages) */
#define KERNEL_STACK_SIZE 2

/* a macro to test if a pointer is inside kernel space */
#define is_kernel_address(x)  (((addr_t)(x)) & KERNEL_ADDR_MASK)


/*** User Memory Layout ***/

/* User base address */
#define USER_BASE 0x00000000

/* User space size */
#define USER_SIZE 0xC0000000

/* User space top address */
#define USER_TOP (USER_SPACE + USER_SIZE - 1)

/* a macro to test if a pointer is insude user space */
#define is_user_address(x) (((addr_t)(x)) >= USER_BASE && ((addr_t)(x)) <= USER_TOP)


#endif
