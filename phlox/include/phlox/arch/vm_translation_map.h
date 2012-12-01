/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_VM_TRANSLATION_MAP_H
#define _PHLOX_ARCH_VM_TRANSLATION_MAP_H

#include <arch/arch_bits.h>
#include <arch/arch_data.h>
#include <phlox/types.h>
#include <phlox/ktypes.h>
#include <phlox/kargs.h>
#include <phlox/spinlock.h>
#include <phlox/list.h>
#include INC_ARCH(phlox/arch,vm_translation_map.h)


/* VM translation map object */
typedef struct vm_translation_map_struct {
    /* List node */
    list_elem_t                           list_node;
    /* Set of translation map operations */
    struct vm_translation_map_ops_struct  *ops;
    /* Access lock. Note: Spinlock must be replaced with more complex lock! */
    spinlock_t                            lock;
    /* Mapped size */
    uint                                  map_count;
    /* Architecture-dependend data */
    arch_vm_translation_map_t             arch;
} vm_translation_map_t;

/* VM translation map operations */
typedef struct vm_translation_map_ops_struct {
    void     (*destroy)(vm_translation_map_t *tmap);
    status_t (*lock)(vm_translation_map_t *tmap);
    status_t (*unlock)(vm_translation_map_t *tmap);
    status_t (*map)(vm_translation_map_t *tmap, addr_t va, addr_t pa, uint protection);
    status_t (*unmap)(vm_translation_map_t *tmap, addr_t start, addr_t end);
    status_t (*query)(vm_translation_map_t *tmap, addr_t va, addr_t *out_pa, uint *out_flags);
    size_t   (*get_mapped_size)(vm_translation_map_t *tmap);
    status_t (*protect)(vm_translation_map_t *tmap, addr_t start, addr_t end, uint protection);
    status_t (*clear_flags)(vm_translation_map_t *tmap, addr_t va, uint flags);
    void     (*flush)(vm_translation_map_t *tmap);
    status_t (*get_physical_page)(addr_t pa, addr_t *out_va, uint flags);
    status_t (*put_physical_page)(addr_t va);
} vm_translation_map_ops_t;


/*
 * Translation map module initialization routine.
 * Called during virtual memory initialization.
*/
status_t vm_translation_map_init(kernel_args_t *kargs);

/*
 * Translation map module final stage of initialization.
 * Called during final stage of VM initialization.
*/
status_t vm_translation_map_init_final(kernel_args_t *kargs);

/*
 * Map a page directly without using any of Virtual Memory Manager objects.
 * This routine used only during system start up. Do not use after.
*/
status_t vm_tmap_quick_map_page(kernel_args_t *kargs, addr_t va, addr_t pa, uint protection);

/*
 * Returns physical address of a page overriding Virtual Memory Manager.
 * This routine used only during system start up. Do not use after.
 */
status_t vm_tmap_quick_query(addr_t vaddr, addr_t *out_paddr);

/*
 * Create translation map object for kernel address space.
 * This routine used only during system start up. Do not use after.
 */
status_t vm_tmap_kernel_create(vm_translation_map_t *kernel_tmap);

/*
 * Create translation map object for user-space.
 */
status_t vm_tmap_create(vm_translation_map_t *new_tmap);

#endif
