/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_I386_CPU_H
#define _PHLOX_ARCH_I386_CPU_H

/* PAGE_SHIFT determines the memory page size */
#define PAGE_SHIFT  12
#define PAGE_SIZE   (1UL << PAGE_SHIFT)
#define PAGE_MASK   (~(PAGE_SIZE-1))

/* Align the pointer to the (next) page boundary */
#define PAGE_ALIGN(a)   (((a)+PAGE_SIZE-1)&PAGE_MASK)

/* Max. limit of Global Descriptor Table */
#define MAX_GDT_LIMIT  0x800
/*
* NOTE: Global Descroptor Table limit can be up to 0x10000 bytes,
* but Phlox uses only 0x800 (256 entries).
*/

/* Max. limit of Interrupt Descriptor Table */
#define MAX_IDT_LIMIT  0x800

#endif
