/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_KERNEL_ARGS_H_
#define _PHLOX_KERNEL_ARGS_H_

#include <phlox/ktypes.h>
#include <phlox/arch/kargs.h>

#define MAX_PHYS_MEM_ADDR_RANGE   16
#define MAX_PHYS_ALLOC_ADDR_RANGE 16
#define MAX_VIRT_ALLOC_ADDR_RANGE 16

/* Magic value */
#define KARGS_MAGIC 'karg'

/* Kernel args (used for pass boot params into kernel before it starts) */
typedef struct {
    uint32 magic;                   /* This field used for verification */
    uint32 cons_line;
    addr_range_t btfs_image_addr;   /* Physical address range of BootFS image */
    addr_range_t phys_kernel_addr;  /* Physical address range of kernel image */
    addr_range_t virt_kernel_addr;  /* Virtual address range of kernel image */
    addr_t memsize;                 /* Total memory size */
    /* Ranges of physical memory */
    uint32       num_phys_mem_ranges;
    addr_range_t phys_mem_range[MAX_PHYS_MEM_ADDR_RANGE];
    /* Ranges of allocated physical memory */
    uint32       num_phys_alloc_ranges;
    addr_range_t phys_alloc_range[MAX_PHYS_ALLOC_ADDR_RANGE];
    /* Ranges of allocated virtual memory */
    uint32     num_virt_alloc_ranges;
    addr_range_t virt_alloc_range[MAX_VIRT_ALLOC_ADDR_RANGE];
    
    uint32 num_cpus;                               /* Number of CPUs */
    addr_range_t phys_cpu_kstack[SYSCFG_MAX_CPUS]; /* Physical stack ranges */
    addr_range_t virt_cpu_kstack[SYSCFG_MAX_CPUS]; /* Virtual stack ranges */

    /* framebuffer stuff */
    struct framebuffer {
        uint32 enabled;
        uint32 x_size;
        uint32 y_size;
        uint32 bit_depth;
        uint8  red_mask_size;
        uint8  red_field_position;
        uint8  green_mask_size;
        uint8  green_field_position;
        uint8  blue_mask_size;
        uint8  blue_field_position;
        uint8  reserved_mask_size;
        uint8  reserved_field_position;
        uint32 already_mapped;
        addr_range_t mapping;
        addr_range_t phys_addr;
    } fb;

    /* architecture specific */
    arch_kernel_args_t arch_args;
} kernel_args_t;

#endif
