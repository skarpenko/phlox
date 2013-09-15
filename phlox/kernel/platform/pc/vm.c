/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/types.h>
#include <phlox/kernel.h>
#include <phlox/kargs.h>
#include <arch/arch_data.h>
#include <arch/arch_bits.h>
#include <phlox/errors.h>
#include <phlox/arch/vm_translation_map.h>
#include <phlox/vm_page.h>
#include <phlox/vm.h>
#include <phlox/vm_names.h>


/* Platform-specific definitions */
#define DMA_AREA_BASE      0x00000
#define DMA_AREA_SIZE      0xa0000
#define SCREEN_AREA_BASE   0xb8000
#define SCREEN_AREA_SIZE   PAGE_SIZE


/* platform-specific init for virtual memory manager */
status_t platform_vm_init(kernel_args_t *kargs)
{
    return NO_ERROR;
}

/* prefinal stage of platform-specific init */
status_t platform_vm_init_prefinal(kernel_args_t *kargs)
{
    /* mark DMA IO area as in use */
    vm_page_mark_range_inuse(DMA_AREA_BASE / PAGE_SIZE, DMA_AREA_SIZE / PAGE_SIZE);

    /* mark screen area as in use */
    vm_page_mark_range_inuse(SCREEN_AREA_BASE / PAGE_SIZE, SCREEN_AREA_SIZE / PAGE_SIZE);

    return NO_ERROR;
}

/* final stage of platform-specific init */
status_t platform_vm_init_final(kernel_args_t *kargs)
{
    object_id id;

    /* create DMA area object */
    id = vm_create_physmem_object(VM_NAME_PC_KERNEL_DMA_AREA, DMA_AREA_BASE,
                                  DMA_AREA_SIZE, VM_OBJECT_PROTECT_ALL);
    if(id == VM_INVALID_OBJECTID)
        return ERR_GENERAL;

    /* create screen area object */
    id = vm_create_physmem_object(VM_NAME_PC_KERNEL_SCREEN_AREA, SCREEN_AREA_BASE,
                                  SCREEN_AREA_SIZE, VM_OBJECT_PROTECT_ALL);
    if(id == VM_INVALID_OBJECTID)
        return ERR_GENERAL;

    return NO_ERROR;
}
