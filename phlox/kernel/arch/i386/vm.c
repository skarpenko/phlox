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
#include <phlox/vm_private.h>


/* architecture-specific init for virtual memory manager */
status_t arch_vm_init(kernel_args_t *kargs)
{
    return NO_ERROR;
}

/* prefinal stage of architecture-specific init */
status_t arch_vm_init_prefinal(kernel_args_t *kargs)
{
    return NO_ERROR;
}

/* final stage of architecture-specific init */
status_t arch_vm_init_final(kernel_args_t *kargs)
{
    aspace_id kid = vm_get_kernel_aspace_id();
    object_id id;
    status_t err;

    /* create GDT object and its mapping */
    id = vm_create_physmem_object(VM_NAME_I386_KERNEL_GDT, kargs->arch_args.phys_gdt,
                                  PAGE_SIZE, VM_OBJECT_PROTECT_ALL);
    if(id == VM_INVALID_OBJECTID)
        return ERR_GENERAL;

    err = vm_map_object_exactly(kid, id, VM_PROT_KERNEL_ALL, kargs->arch_args.virt_gdt);
    if(err != NO_ERROR)
        return err;

    /* init page reference counter */
    err = vm_page_init_wire_counters(kargs->arch_args.virt_gdt, PAGE_SIZE);
    if(err != NO_ERROR)
        return err;

    /* create IDT object and its mapping */
    id = vm_create_physmem_object(VM_NAME_I386_KERNEL_IDT, kargs->arch_args.phys_idt,
                                  PAGE_SIZE, VM_OBJECT_PROTECT_ALL);
    if(id == VM_INVALID_OBJECTID)
        return ERR_GENERAL;

    err = vm_map_object_exactly(kid, id, VM_PROT_KERNEL_ALL, kargs->arch_args.virt_idt);
    if(err != NO_ERROR)
        return err;

    /* init page reference counter */
    err = vm_page_init_wire_counters(kargs->arch_args.virt_idt, PAGE_SIZE);

    return NO_ERROR;
}
