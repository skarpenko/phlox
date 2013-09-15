/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_I386_VM_NAMES_H_
#define _PHLOX_ARCH_I386_VM_NAMES_H_


/* Global Descriptor Table object */
#define VM_NAME_I386_KERNEL_GDT                "kernel_gdt"
/* Interrupt Descriptor Table object */
#define VM_NAME_I386_KERNEL_IDT                "kernel_idt"
/* Kernel page directory object */
#define VM_NAME_I386_KERNEL_PAGE_DIR           "kernel_page_dir"
/* Object of per CPU Task State Segments */
#define VM_NAME_I386_KERNEL_TSS                "kernel_tss"
/* Object of page tables used by mapping pool */
#define VM_NAME_I386_KERNEL_MAP_POOL_PGTABLES  "kernel_map_pool_pgtables"


#endif
