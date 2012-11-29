/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_VM_TRANSLATION_MAP_H
#define _PHLOX_ARCH_VM_TRANSLATION_MAP_H

#include INC_ARCH(phlox/arch,vm_translation_map.h)

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
status_t vm_tmap_quick_map_page(kernel_args_t *kargs, addr_t va, addr_t pa, uint attributes);

/*
 * Returns physical address of a page overriding Virtual Memory Manager.
 * This routine used only during system start up. Do not use after.
 */
status_t vm_tmap_quick_query(addr_t vaddr, addr_t *out_paddr);

#endif
