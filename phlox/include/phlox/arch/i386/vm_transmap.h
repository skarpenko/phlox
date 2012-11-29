/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_I386_VM_TRANSMAP_H
#define _PHLOX_ARCH_I386_VM_TRANSMAP_H

/* Some i386 architecture specific definitions */
#define MAX_PDENTS  1024
#define MAX_PTENTS  1024

/* Some useful macroses */
#define ADDR_SHIFT(x)          ((x)>>PAGE_SHIFT)
#define ADDR_REVERSE_SHIFT(x)  ((x)<<PAGE_SHIFT)
/* virtual address to page directory entry */
#define VADDR_TO_PDENT(va)  (((va) / PAGE_SIZE) / MAX_PDENTS)
/* virtual address to page table entry */
#define VADDR_TO_PTENT(va)  (((va) / PAGE_SIZE) % MAX_PTENTS)

#endif
