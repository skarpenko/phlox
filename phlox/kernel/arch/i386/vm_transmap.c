/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <arch/arch_bits.h>
#include <arch/arch_data.h>
#include <phlox/types.h>
#include <phlox/kernel.h>
#include <phlox/kargs.h>
#include <phlox/processor.h>
#include <phlox/arch/vm_transmap.h>
#include <phlox/vm_page.h>
#include <phlox/vm.h>

/* init page directory entry */
static inline void init_pdentry(mmu_pde *pdentry)
{
   pdentry->raw.dword0 = 0;
}

/* init page table entry */
static inline void init_ptentry(mmu_pte *ptentry)
{
   ptentry->raw.dword0 = 0;
}

/* put page table into page directory entry */
static void put_pgtable_in_pgdir(mmu_pde *pdentry, addr_t pgtable_phys, uint32 attributes)
{
    /* put it in the page directory */
    init_pdentry(pdentry);
    pdentry->stru.base = ADDR_SHIFT(pgtable_phys);

    /* mark it user and read/write.
     * The page table entries will override it.
     */
    pdentry->stru.us = 1;
    pdentry->stru.rw = 1;
    pdentry->stru.p  = 1;
}

/* Allocate page hole and return index of page directory entry.
 * This routine makes recursive page directory entry in pgdir.
 * Set inv_ar = true if TLB invalidation required for page hole
 * address range.
 */
static uint32 allocate_page_hole(mmu_pde *pgdir, addr_t phys_pgdir_addr, bool inv_ar)
{
    uint32 i;
    uint32 res = 0; /* zero returns on fail */

    /* search from the end of page directory */
    for(i=MAX_PDENTS-1; i>0; i--) {
        /* if entry is not used */
        if(!pgdir[i].stru.p) {
           /* init entry */
           init_pdentry(&pgdir[i]);
           /* make it recursively refer to page directory */
           pgdir[i].stru.base = ADDR_SHIFT(phys_pgdir_addr);
           /* set flags */
           pgdir[i].stru.us = 1;
           pgdir[i].stru.rw = 1;
           pgdir[i].stru.p  = 1;
           /* invalidate page hole address range if requested */
           if(inv_ar)
             invalidate_TLB_range(i * MAX_PDENTS * PAGE_SIZE, MAX_PDENTS * PAGE_SIZE);
           res = i; /* set return result */
           break;
        }
    }
    /* return index if page directory entry or zero
     * if no free entries in page directory. so... zero index
     * is reserved.
     */
    return res;
}

/* Dellocate previously allocated page hole. */
static void deallocate_page_hole(mmu_pde *pgdir, uint32 pde_idx, bool inv_ar)
{
    /* remove entry from page directory */
    pgdir[pde_idx].raw.dword0 = 0;
    /* invalidate page hole address range if requested */
    if(inv_ar)
      invalidate_TLB_range(pde_idx * MAX_PDENTS * PAGE_SIZE, MAX_PDENTS * PAGE_SIZE);
}

/* init translation map module */
uint32 arch_vm_transmap_init(kernel_args_t *kargs)
{
    uint32 pghole_pde_idx;
    mmu_pte *pghole;
    mmu_pde *pghole_pgdir;

    /* first of all, allocate page hole */
    pghole_pde_idx = allocate_page_hole((mmu_pde *)kargs->arch_args.virt_pgdir,
                                      kargs->arch_args.phys_pgdir, true);
    if(!pghole_pde_idx)
        panic("arch_vm_transmap_init: page hole allocation failed. :(");

    /* page hole address */
    pghole = (mmu_pte *)PDENT_TO_VADDR(pghole_pde_idx);
    /* page directory address within page hole */
    pghole_pgdir = (mmu_pde *)((uint32)pghole + pghole_pde_idx * PAGE_SIZE);
     
    /* and now, set global bit to all pages allocated in kernel space */
    {
      uint32 kbase_pde = VADDR_TO_PDENT(KERNEL_BASE);  /* kernel base pd entry */
      uint32 kbase_pte = VADDR_TO_PTENT(KERNEL_BASE);  /* kernel base pt entry */
      mmu_pte *pgtable;   /* page table */
      uint32 i, j, first_pgent;

      /* walk through page directory entries */
      for(i=kbase_pde; i<MAX_PDENTS; i++) {
         /* ignore recursive page directory entry */
         if(i==pghole_pde_idx) continue;
         /* if page table exists */
         if(pghole_pgdir[i].stru.p) {
             /*
              * if kernel base is not aligned to page directory entry
              * use the only part of page table that referes to kernel
              * space.
              */
             if(i==kbase_pde)
               /* set proper page table entry index if
                * current page directory entry is the first
                * refered to kernel base.
                */
               first_pgent = kbase_pte;
             else
               first_pgent = 0; /* else start from first page table entry */

             /* get page table address from page hole */
             pgtable = (mmu_pte *)((uint32)pghole + i * PAGE_SIZE);
             /* walk throught page table entries */
             for(j=first_pgent; j<MAX_PTENTS; j++) {
               if(pgtable[j].stru.p) {
                  /* if page present, set global bit and invalidate
                   * its virtual address
                   */
                  pgtable[j].stru.g = 1;
                  invalidate_TLB_entry( PDPTENT_TO_VADDR(i, j) );
               }
             }
         }
      }
      /* Turn on Global bit if supported.
       * Note that only bootstrap processor enters this section during
       * kernel initialization stage.
       */
      if(ProcessorSet.processors[BOOTSTRAP_CPU].arch.features[I386_FEATURE_D] & X86_CPUID_PGE) {
          uint32 cr4; /* temp storage for CR4 register */
          cr4 = read_cr4();
          write_cr4(cr4 | X86_CR4_PGE); /* Set PGE bit in CR4 */
      }
    } /* end of Set Global Bit block */

    /* deallocate page hole */
    deallocate_page_hole((mmu_pde *)kargs->arch_args.virt_pgdir, pghole_pde_idx, true);

    return 0;
}

/*
 * Map a page directly without using any of Virtual Memory Manager objects.
 * Uses a 'page hole' also known as 'recursive directory entry'. The page hole is created by pointing
 * one of the page directory entries back at itself, effectively mapping the contents of all of the
 * 4Mb of page tables into a 4Mb region.
 * This routine used only during system start up. Do not use after.
 */
uint32 vm_transmap_quick_map_page(kernel_args_t *kargs, addr_t virt_addr, addr_t phys_addr, uint32 attributes)
{
    mmu_pte *pgentry;
    uint32 pghole_pde_idx;
    mmu_pte *page_hole;
    mmu_pde *page_hole_pgdir;
    uint32 pgtable_idx;

    /* allocate page hole */
    pghole_pde_idx = allocate_page_hole((mmu_pde *)kargs->arch_args.virt_pgdir,
                                      kargs->arch_args.phys_pgdir, true);
    if(!pghole_pde_idx)
        panic("vm_transmap_quick_map: page hole allocation failed. :(");

    /* page hole address */
    page_hole = (mmu_pte *)PDENT_TO_VADDR(pghole_pde_idx);
    /* page directory address within page hole */
    page_hole_pgdir = (mmu_pde *)((uint32)page_hole + pghole_pde_idx * PAGE_SIZE);
        
    /* check to see if a page table exists for this range */
    pgtable_idx = VADDR_TO_PDENT(virt_addr);
    if(!page_hole_pgdir[pgtable_idx].stru.p) {
        addr_t pgtable;
        mmu_pde *pdentry;
        /* we need to allocate a page table */
        pgtable = vm_alloc_phpage_from_kargs(kargs);
        if(!pgtable)
           panic("vm_transmap_quick_map: failed to allocate physical page.");
        /* page table is in pages, convert to physical address */
        pgtable *= PAGE_SIZE;

        /* put it in the pgdir */
        pdentry = &page_hole_pgdir[pgtable_idx];
        put_pgtable_in_pgdir(pdentry, pgtable, attributes);

        /* zero it out in it's new mapping */
        memset((uint32 *)((uint32)page_hole + VADDR_TO_PDENT(virt_addr) * PAGE_SIZE), 0, PAGE_SIZE);
    }
    /* now, fill in the page entry */
    pgentry = page_hole + virt_addr / PAGE_SIZE;
    init_ptentry(pgentry);
    /* map physical address to virtual */
    pgentry->stru.base = ADDR_SHIFT(phys_addr);
    /* set page attributes */
    pgentry->stru.us = !(attributes & VM_LOCK_KERNEL);
    pgentry->stru.rw = attributes & VM_LOCK_RW;
    pgentry->stru.p  = 1;
    if(is_kernel_address(virt_addr))
        pgentry->stru.g = 1; /* global bit set for all kernel addresses */

    /* deallocate page hole */
    deallocate_page_hole((mmu_pde *)kargs->arch_args.virt_pgdir, pghole_pde_idx, true);

    /* invalidate newly mapped address */
    invalidate_TLB_entry(virt_addr);

    return 0;
}
