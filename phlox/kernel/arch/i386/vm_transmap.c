/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <arch/arch_bits.h>
#include <arch/arch_data.h>
#include <phlox/types.h>
#include <phlox/kernel.h>
#include <phlox/kargs.h>
#include <phlox/processor.h>
#include <phlox/arch/vm_transmap.h>
#include <phlox/vm_page.h>
#include <phlox/vm.h>

static inline void init_pdentry(mmu_pde *pdentry) {
   pdentry->raw.dword0 = 0;
}

static inline void init_ptentry(mmu_pte *ptentry) {
   ptentry->raw.dword0 = 0;
}

static addr_t vm_transmap_allocate_page_hole(mmu_pde *pde, addr_t phys_pde_addr) {
    uint32 i;
    addr_t res = 0;

    for(i=MAX_PDENTS-1; i>0; i--) {
        if(!pde[i].stru.p) {
           init_pdentry(&pde[i]);
           pde[i].stru.base = ADDR_SHIFT(phys_pde_addr);
           pde[i].stru.us = 1;
           pde[i].stru.rw = 1;
           pde[i].stru.p = 1;
           res = i * MAX_PDENTS * PAGE_SIZE;
           invalidate_TLB_entry(res);
	   break;
        }
    }

    return res;
}


/*
 * Map a page directly without using any of Virtual Memory Manager objects.
 * Uses a 'page hole' also known as 'recursive directory entry'. The page hole is created by pointing
 * one of the page directory entries back at itself, effectively mapping the contents of all of the
 * 4Mb of page tables into a 4Mb region.
 * This routine used only during system start up. Do not use after.
 */
int vm_transmap_quick_map(kernel_args_t *kargs, addr_t va, addr_t pa, uint32 attributes) {
    uint32 res;
    res = vm_transmap_allocate_page_hole(kargs->arch_args.virt_pgdir, kargs->arch_args.phys_pgdir);
    kprint("Page Hole At 0x%X\n", res);
}

