/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <sys/debug.h>
#include <arch/arch_bits.h>
#include <arch/arch_data.h>
#include <phlox/types.h>
#include <phlox/errors.h>
#include <phlox/kernel.h>
#include <phlox/kargs.h>
#include <phlox/processor.h>
#include <phlox/atomic.h>
#include <phlox/spinlock.h>
#include <phlox/list.h>
#include <phlox/heap.h>
#include <phlox/arch/vm_translation_map.h>
#include <phlox/vm_private.h>
#include <phlox/vm_page.h>
#include <phlox/vm.h>
#include <phlox/vm_names.h>
#include <phlox/vm_page_mapper.h>

/* Kernel's page directory */
static mmu_pde *kernel_pgdir_phys = NULL; /* Physical address */
static mmu_pde *kernel_pgdir_virt = NULL; /* Virtual address  */

/* Page hole hack */
static mmu_pte *page_hole = NULL;  /* Page hole address      */
static mmu_pde *page_hole_pgdir;   /* Page directory address */
static uint     page_hole_pdeidx;  /* Page hole's PDE index  */

/* Translation maps list */
static xlist_t    tmap_list;       /* List */
static spinlock_t tmap_list_lock;  /* Spinlock for list access */

/* Mapping pool */
static addr_t  map_pool_base;      /* Mapping pool base address    */
static mmu_pte *map_pool_pgtables; /* Page tables for mapping pool */
/* each page table covers 4Mb of space in mapping pool */
#define MAP_POOL_PGTABLE_CHUNK_SIZE  (MAX_PTENTS * PAGE_SIZE)
/* Number of page tables per mapping pool */
#define MAP_POOL_PGTABLE_CHUNKS      8
/* Mapping pool size */
#define MAP_POOL_SIZE                (MAP_POOL_PGTABLE_CHUNKS * MAP_POOL_PGTABLE_CHUNK_SIZE)

/* Locally used constants */
#define FIRST_USER_PGDIR_ENTRY       (VADDR_TO_PDENT(USER_BASE))
#define NUM_USER_PGDIR_ENTRIES       (VADDR_TO_PDENT(ROUNDUP(USER_SIZE, PAGE_SIZE * MAX_PTENTS)))
#define FIRST_KERNEL_PGDIR_ENTRY     (VADDR_TO_PDENT(KERNEL_BASE))
#define NUM_KERNEL_PGDIR_ENTRIES     (VADDR_TO_PDENT(KERNEL_SIZE))

/* Translation map operations */
static void destroy_tmap(vm_translation_map_t *tmap);
static status_t lock_tmap(vm_translation_map_t *tmap);
static status_t unlock_tmap(vm_translation_map_t *tmap);
static status_t map_tmap(vm_translation_map_t *tmap, addr_t va, addr_t pa, uint protection);
static status_t unmap_tmap(vm_translation_map_t *tmap, addr_t start, addr_t end);
static status_t query_tmap(vm_translation_map_t *tmap, addr_t va, addr_t *out_pa, uint *out_flags);
static size_t get_mapped_size_tmap(vm_translation_map_t *tmap);
static status_t protect_tmap(vm_translation_map_t *tmap, addr_t start, addr_t end, uint protection);
static status_t clear_flags_tmap(vm_translation_map_t *tmap, addr_t va, uint flags);
static void flush_tmap(vm_translation_map_t *tmap);
static status_t get_physical_page_tmap(addr_t pa, addr_t *out_va, uint flags);
static status_t put_physical_page_tmap(addr_t va);

/* Structured translation map operations */
static vm_translation_map_ops_t ops_tmap = {
    destroy_tmap,            /* Destroy translation map                */
    lock_tmap,               /* Acquire lock on translation map access */
    unlock_tmap,             /* Release lock on translation map access */
    map_tmap,                /* Map physical address                   */
    unmap_tmap,              /* Unmap virtual address range            */
    query_tmap,              /* Query physical address                 */
    get_mapped_size_tmap,    /* Get mapped size                        */
    protect_tmap,            /* Set protection attributes              */
    clear_flags_tmap,        /* Clear page flags                       */
    flush_tmap,              /* Flush internal page cache              */
    get_physical_page_tmap,  /* Get physical page via mappings pool    */
    put_physical_page_tmap   /* Put physical page                      */
};


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

/* update page directories in all translation maps */
static void update_all_pgdirs(uint index, mmu_pde pdentry)
{
    unsigned long irqs_state;
    list_elem_t *le;
    vm_translation_map_t *tm_e;

    /* acquire lock before performing modifications */
    irqs_state = spin_lock_irqsave(&tmap_list_lock);

    /* walk throught list and update all translation maps */
    le = xlist_peek_first(&tmap_list);
    while (le) {
        tm_e = containerof(le, vm_translation_map_t, list_node);
        tm_e->arch.pgdir_virt[index] = pdentry;
        
        le = le->next; /* next item */
    }

    /* release lock */
    spin_unlock_irqrstor(&tmap_list_lock, irqs_state);
}

/* put page table into page directory entry */
static void put_pgtable_in_pgdir(mmu_pde *pdentry, addr_t pgtable_phys, uint protection)
{
    /* put it in the page directory */
    init_pdentry(pdentry);
    pdentry->stru.base = ADDR_SHIFT(pgtable_phys);

    /* mark it system and read/write.
     * The page table entries will override it.
     */
    pdentry->stru.us = !(protection & VM_PROT_KERNEL) != 0;
    pdentry->stru.rw = (protection & VM_PROT_WRITE) != 0;
    pdentry->stru.p  = 1;
}

/* destroy translation map */
static void destroy_tmap(vm_translation_map_t *tmap)
{
    unsigned long irqs_state;
    unsigned int i;

    /* exit if NULL */
    if(tmap == NULL)
        return;

    /* acquire spinlock and remove tmap from tmaps list */
    irqs_state = spin_lock_irqsave(&tmap_list_lock);
    xlist_remove(&tmap_list, &tmap->list_node);
    spin_unlock_irqrstor(&tmap_list_lock, irqs_state);

    if(tmap->arch.pgdir_virt != NULL) {
        /* cycle through and free all of the user space pgtables */
        for(i = VADDR_TO_PDENT(USER_BASE);
                i <= VADDR_TO_PDENT(USER_BASE + (USER_SIZE - 1)); i++) {
            addr_t pgtable_addr;
            vm_page_t *page;

            /* if page table present in memory
             * mark pages it refers as free
             */
            if(tmap->arch.pgdir_virt[i].stru.p == 1) {
                pgtable_addr = tmap->arch.pgdir_virt[i].stru.base;
                page = vm_page_lookup(pgtable_addr);
                if(!page)
                    panic("destroy_tmap: didn't find pgtable page\n");
                vm_page_set_state(page, VM_PAGE_STATE_FREE);
            }
        }
        kfree(tmap->arch.pgdir_virt);
        tmap->arch.pgdir_virt = NULL;
    }

    /* unlink translation map operations */
    tmap->ops = NULL;

    /* TODO: other clean ops goes here */
}

/* acquire translation map access lock */
static status_t lock_tmap(vm_translation_map_t *tmap)
{
/** WARNING: interrupts should be disabled due to spinlock. **/
/*
 * TODO: locking mechanisms here should be reimplemented
 *       using more advanced techniques.
 */

    spin_lock(&tmap->lock);
    tmap->arch.num_invalidate_pages = 0;

    return NO_ERROR;
}

/* release translation map access lock */
static status_t unlock_tmap(vm_translation_map_t *tmap)
{
/** WARNING: interrupts should be disabled due to spinlock. **/
/*
 * TODO: locking mechanisms here should be reimplemented
 *       using more advanced techniques.
 */

    flush_tmap(tmap);
    spin_unlock(&tmap->lock);

    return NO_ERROR;
}

/* map physical address */
static status_t map_tmap(vm_translation_map_t *tmap, addr_t va, addr_t pa, uint protection)
{
    mmu_pde *pgdir = pgdir = tmap->arch.pgdir_virt;
    mmu_pte *pgtbl;
    unsigned int index;
    vm_page_t *page;
    status_t status;

    /* check to see if a page table exists for this range */
    index = VADDR_TO_PDENT(va);
    if(pgdir[index].stru.p == 0) {
        addr_t pgtable;

        /* we need to allocate a pgtable */
        page = vm_page_alloc(VM_PAGE_STATE_CLEAR);

        /* mark the page WIRED */
        vm_page_set_state(page, VM_PAGE_STATE_WIRED);
        
        pgtable = page->ppn * PAGE_SIZE;

        /* put it in the page directory */
        put_pgtable_in_pgdir(&pgdir[index], pgtable, protection | VM_PROT_READ | VM_PROT_WRITE);

        /* update any other page directories, if it maps kernel space */
        if(index >= FIRST_KERNEL_PGDIR_ENTRY &&
           index < (FIRST_KERNEL_PGDIR_ENTRY + NUM_KERNEL_PGDIR_ENTRIES)) {
            update_all_pgdirs(index, pgdir[index]);
        }

        tmap->map_count++;
    }

    /* now, fill in the page table entry */
    do {
        /* get page from via mappings pool */
        status = get_physical_page_tmap(ADDR_REVERSE_SHIFT(pgdir[index].stru.base),
                                        (addr_t *)&pgtbl, false);
    } while(status != NO_ERROR);

    index = VADDR_TO_PTENT(va);

    /* init page table entry */
    init_ptentry(&pgtbl[index]);
    pgtbl[index].stru.base = ADDR_SHIFT(pa);
    pgtbl[index].stru.us = !(protection & VM_PROT_KERNEL) != 0;
    pgtbl[index].stru.rw = (protection & VM_PROT_WRITE) != 0;
    pgtbl[index].stru.p = 1;
    if(is_kernel_address(va))
        pgtbl[index].stru.g = 1; /* global bit set for all kernel addresses */

    /* increment wired counter (count of refered maps) */
    page = vm_page_lookup(pgtbl[index].stru.base);
    ASSERT_MSG(page, "map_tmap(): page = NULL!");
    atomic_inc((atomic_t*)&page->wire_count);

    /* put page back */
    put_physical_page_tmap((addr_t)pgtbl);

    /* add page address into invalidation cache */
    if(tmap->arch.num_invalidate_pages < PAGE_INVALIDATE_CACHE_SIZE) {
        tmap->arch.pages_to_invalidate[tmap->arch.num_invalidate_pages] = va;
    }
    tmap->arch.num_invalidate_pages++;

    tmap->map_count++;

    /* all done */
    return NO_ERROR;
}

/* unmap virtual address range */
static status_t unmap_tmap(vm_translation_map_t *tmap, addr_t start, addr_t end)
{
    mmu_pde *pgdir = tmap->arch.pgdir_virt;
    mmu_pte *pgtbl;
    unsigned int index;
    vm_page_t *page;
    status_t status;

    /* align by page size */
    start = ROUNDOWN(start, PAGE_SIZE);
    end = ROUNDUP(end, PAGE_SIZE);

restart:
    if(start >= end)
        return NO_ERROR;

    index = VADDR_TO_PDENT(start);
    if(pgdir[index].stru.p == 0) {
        /* no pagetable here, move the start up to access the next page table */
        start = ROUNDUP(start + 1, PAGE_SIZE);
        goto restart;
    }

    /* get page via mappings pool */
    do {
        status = get_physical_page_tmap(ADDR_REVERSE_SHIFT(pgdir[index].stru.base),
                                        (addr_t *)&pgtbl, false);
    } while(status != NO_ERROR);

    /* unmap pages from page table */
    for(index = VADDR_TO_PTENT(start); (index < MAX_PTENTS) && (start < end); index++, start += PAGE_SIZE) {
        if(pgtbl[index].stru.p == 0) {
            /* page mapping not valid */
            continue;
        }

        pgtbl[index].stru.p = 0; /* mark as not present */
        tmap->map_count--;

        /* decrement wired counter (count of refered maps) */
        page = vm_page_lookup(pgtbl[index].stru.base);
        ASSERT_MSG(page, "unmap_tmap(): page = NULL!");
        atomic_dec((atomic_t*)&page->wire_count);

        /* add page address into invalidation cache */
        if(tmap->arch.num_invalidate_pages < PAGE_INVALIDATE_CACHE_SIZE) {
            tmap->arch.pages_to_invalidate[tmap->arch.num_invalidate_pages] = start;
        }
        tmap->arch.num_invalidate_pages++;
    }

    /* put page back */
    put_physical_page_tmap((addr_t)pgtbl);

    /* jump to next step */
    goto restart;
}

/* query physical address */
static status_t query_tmap(vm_translation_map_t *tmap, addr_t va, addr_t *out_pa, uint *out_flags)
{
    mmu_pde *pgdir = tmap->arch.pgdir_virt;
    mmu_pte *pgtbl;
    unsigned int index;
    status_t status;

    /* default the flags to not present */
    *out_flags = 0;
    *out_pa = 0;

    index = VADDR_TO_PDENT(va);
    if(pgdir[index].stru.p == 0) {
        /* no pagetable here */
        return NO_ERROR;
    }

    /* get page via mappings pool */
    do {
        status = get_physical_page_tmap(ADDR_REVERSE_SHIFT(pgdir[index].stru.base),
                                        (addr_t *)&pgtbl, false);
    } while(status != NO_ERROR);
    index = VADDR_TO_PTENT(va);

    /* return physical address to caller */
    *out_pa = ADDR_REVERSE_SHIFT(pgtbl[index].stru.base) | (va & (PAGE_SIZE-1));

    /* read in the page state flags */
    *out_flags = 0;
    *out_flags |= pgtbl[index].stru.rw ? (VM_PROT_READ | VM_PROT_WRITE) : VM_PROT_READ;
    *out_flags |= pgtbl[index].stru.us ? 0 : VM_PROT_KERNEL;
    *out_flags |= VM_PROT_EXECUTE; /* all pages is executable on IA-32 */
    *out_flags |= pgtbl[index].stru.d ? VM_FLAG_PAGE_MODIFIED : 0;
    *out_flags |= pgtbl[index].stru.a ? VM_FLAG_PAGE_ACCESSED : 0;
    *out_flags |= pgtbl[index].stru.p ? VM_FLAG_PAGE_PRESENT : 0;

    /* put page back */
    put_physical_page_tmap((addr_t)pgtbl);

    /* all done */
    return NO_ERROR;
}

/* return mapped size */
static size_t get_mapped_size_tmap(vm_translation_map_t *tmap)
{
    return tmap->map_count;
}

/* set protection flags */
static status_t protect_tmap(vm_translation_map_t *tmap, addr_t start, addr_t end, uint protection)
{
    mmu_pte *pgtbl;
    mmu_pde *pgdir = tmap->arch.pgdir_virt;
    unsigned int index;
    status_t status;

    /* align by page size */
    start = ROUNDOWN(start, PAGE_SIZE);
    end = ROUNDUP(end, PAGE_SIZE);

restart:
    if (start >= end)
        return NO_ERROR;

    index = VADDR_TO_PDENT(start);
    if (pgdir[index].stru.p == 0) {
        /* no pagetable here, move the start up to access the next page table */
        start = ROUNDUP(start + 1, PAGE_SIZE);
        goto restart;
    }

    /* get page via mappings pool */
    do {
        status = get_physical_page_tmap(ADDR_REVERSE_SHIFT(pgdir[index].stru.base),
                (addr_t *)&pgtbl, false);
    } while (status != NO_ERROR);

    /* walk through page table */
    for (index = VADDR_TO_PTENT(start); index < MAX_PTENTS && start < end; index++, start += PAGE_SIZE) {
        if (pgtbl[index].stru.p == 0) {
            /* page mapping not valid */
            continue;
        }

        /* set flags */
        pgtbl[index].stru.us = !(protection & VM_PROT_KERNEL) != 0;
        pgtbl[index].stru.rw = (protection & VM_PROT_WRITE) != 0;

        /* put address into invalidation cache */
        if(tmap->arch.num_invalidate_pages < PAGE_INVALIDATE_CACHE_SIZE) {
            tmap->arch.pages_to_invalidate[tmap->arch.num_invalidate_pages] = start;
        }
        tmap->arch.num_invalidate_pages++;
    }

    /* put page back into mappings pool */
    put_physical_page_tmap((addr_t)pgtbl);

    /* jump into next step */
    goto restart;
}

/* clear page flags */
static status_t clear_flags_tmap(vm_translation_map_t *tmap, addr_t va, uint flags)
{
    mmu_pte *pgtbl;
    mmu_pde *pgdir = tmap->arch.pgdir_virt;
    unsigned int index;
    status_t status;
    bool tlb_flush = false;

    index = VADDR_TO_PDENT(va);
    if(pgdir[index].stru.p == 0) {
        /* no pagetable here */
        return NO_ERROR;
    }

    /* get page via mappings pool */
    do {
        status = get_physical_page_tmap(ADDR_REVERSE_SHIFT(pgdir[index].stru.base),
                                        (addr_t *)&pgtbl, false);
    } while(status != NO_ERROR);
    index = VADDR_TO_PTENT(va);

    /* clear out the flags we've been requested to clear */
    if(flags & VM_FLAG_PAGE_MODIFIED) {
        pgtbl[index].stru.d = 0;
        tlb_flush = true;
    }

    if(flags & VM_FLAG_PAGE_ACCESSED) {
        pgtbl[index].stru.a = 0;
        tlb_flush = true;
    }

    /* ok. now put page back. */
    put_physical_page_tmap((addr_t)pgtbl);

    /* insert address into invalidation cache if needed */
    if(tlb_flush) {
        if(tmap->arch.num_invalidate_pages < PAGE_INVALIDATE_CACHE_SIZE) {
            tmap->arch.pages_to_invalidate[tmap->arch.num_invalidate_pages] = va;
        }
        tmap->arch.num_invalidate_pages++;
    }

    /* all done. */
    return NO_ERROR;
}

/* flush internal page cache */
static void flush_tmap(vm_translation_map_t *tmap)
{
    unsigned long irqs_state;

    /* return if no pages for invalidate */
    if(tmap->arch.num_invalidate_pages <= 0)
        return;

    /* save current irqs state and disable them */
    local_irqs_save_and_disable(irqs_state);

    if(tmap->arch.num_invalidate_pages > PAGE_INVALIDATE_CACHE_SIZE) {
      /* if count of pages for invalidate more than cache size then
       * perform global invalidation.
       */
       invalidate_TLB();
    } else {
       /* invalidate pages from cache */
       invalidate_TLB_list(tmap->arch.pages_to_invalidate,
                           tmap->arch.num_invalidate_pages);
    }

    tmap->arch.num_invalidate_pages = 0;
    
    /* restore previous irqs state */
    local_irqs_restore(irqs_state);
}

/* get physical page via mappings pool */
static status_t get_physical_page_tmap(addr_t pa, addr_t *out_va, uint flags)
{
    return vm_pmap_get_ppage(pa, out_va, (bool)flags);
}

/* put physical page back */
static status_t put_physical_page_tmap(addr_t va)
{
    return vm_pmap_put_ppage(va);
}

/* Allocate page hole and return index of page directory entry.
 * This routine makes recursive page directory entry in pgdir.
 * Set inv_ar = true if TLB invalidation required for page hole
 * address range.
 */
static uint allocate_page_hole(mmu_pde *pgdir, addr_t phys_pgdir_addr, bool inv_ar)
{
    uint i;
    uint res = 0; /* zero returns on fail */

    /* search from the end of page directory */
    for(i=MAX_PDENTS-1; i>0; i--) {
        /* if entry is not used */
        if(!pgdir[i].stru.p) {
           /* init entry */
           init_pdentry(&pgdir[i]);
           /* make it recursively refer to page directory */
           pgdir[i].stru.base = ADDR_SHIFT(phys_pgdir_addr);
           /* set flags */
           pgdir[i].stru.us = 0;
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
static void deallocate_page_hole(mmu_pde *pgdir, uint pde_idx, bool inv_ar)
{
    /* remove entry from page directory */
    pgdir[pde_idx].raw.dword0 = 0;
    /* invalidate page hole address range if requested */
    if(inv_ar)
      invalidate_TLB_range(pde_idx * MAX_PDENTS * PAGE_SIZE, MAX_PDENTS * PAGE_SIZE);
}

/* Chunk mapper routine. A workhorse of page mapper. */
static status_t map_pool_chunk(addr_t pa, addr_t va)
{
    mmu_pte *pte;
    addr_t ppn;

    pa &= ~(PAGE_SIZE - 1); /* make sure it's page aligned */
    va &= ~(PAGE_SIZE - 1); /* make sure it's page aligned */
    /* check that address is inside mappings pool */
    if(va < map_pool_base || va >= (map_pool_base + MAP_POOL_SIZE))
        return ERR_VM_GENERAL;

    /* map requested page */
    ppn = ADDR_SHIFT(pa);
    pte = &map_pool_pgtables[(va - map_pool_base)/PAGE_SIZE];
    init_ptentry(pte);
    pte->stru.base = ppn;
    pte->stru.us = 0;
    pte->stru.rw = 1;
    pte->stru.p = 1;

    /* invalidate newly mapped address in TLB */
    invalidate_TLB_entry(va);

    return NO_ERROR;
}

/* Common (for kernel and user) translation map creation routine */
static status_t vm_tmap_common_create(vm_translation_map_t *new_tmap, mmu_pde *pgdir_virt,
                                      mmu_pde *pgdir_phys)
{
    unsigned long irqs_state;

    /* init new tmap object structure */
    new_tmap->ops = &ops_tmap;
    new_tmap->map_count = 0;
    new_tmap->arch.num_invalidate_pages = 0;

    /* init spinlock */
    spin_init(&new_tmap->lock);
    /* NOTE: In future spinlock must be replaced with more complex lock! */

    /* assign specified page directory */
    new_tmap->arch.pgdir_virt = pgdir_virt;
    new_tmap->arch.pgdir_phys = pgdir_phys;
    
    /* zero out the bottom portion of the new page directory */
    memset(new_tmap->arch.pgdir_virt + FIRST_USER_PGDIR_ENTRY, 0,
           NUM_USER_PGDIR_ENTRIES * sizeof(mmu_pde));

    /* acquire lock before list touching */
    irqs_state = spin_lock_irqsave(&tmap_list_lock);

    /* copy the top portion of the kernel page directory into new one */
    memcpy(new_tmap->arch.pgdir_virt + FIRST_KERNEL_PGDIR_ENTRY,
           kernel_pgdir_virt + FIRST_KERNEL_PGDIR_ENTRY,
           NUM_KERNEL_PGDIR_ENTRIES * sizeof(mmu_pde));

    /* insert this new map into the map list */
    xlist_add_first(&tmap_list, &new_tmap->list_node);

    /* release lock */
    spin_unlock_irqrstor(&tmap_list_lock, irqs_state);

    return NO_ERROR;
}


/* init translation map module */
status_t vm_translation_map_init(kernel_args_t *kargs)
{
    /* init kernel's page directory pointers */
    kernel_pgdir_phys = (mmu_pde *)kargs->arch_args.phys_pgdir;
    kernel_pgdir_virt = (mmu_pde *)kargs->arch_args.virt_pgdir;

    /* init translation maps list */
    xlist_init(&tmap_list);
    spin_init(&tmap_list_lock);

    /* first of all, allocate page hole */
    page_hole_pdeidx = allocate_page_hole(kernel_pgdir_virt, (addr_t)kernel_pgdir_phys, true);
    if(!page_hole_pdeidx)
        panic("arch_vm_translation_map_init: page hole allocation failed. :(");

    /* page hole address */
    page_hole = (mmu_pte *)PDENT_TO_VADDR(page_hole_pdeidx);
    /* page directory address within page hole */
    page_hole_pgdir = (mmu_pde *)((uint32)page_hole + page_hole_pdeidx * PAGE_SIZE);


    /* Init page mapping engine */
    if( vm_page_mapper_init(kargs, &map_pool_base, MAP_POOL_SIZE, PAGE_SIZE,
                            MAP_POOL_PGTABLE_CHUNK_SIZE, map_pool_chunk) ) {
        panic("arch_vm_translation_map_init: page mapper init failed.");
    }

    /* allocate area for set of page tables which covers mappings pool */
    map_pool_pgtables = (mmu_pte *)vm_alloc_from_kargs( kargs,
                                                        MAP_POOL_PGTABLE_CHUNKS * PAGE_SIZE,
                                                        VM_PROT_KERNEL_DEFAULT);
    if(!map_pool_pgtables)
        panic("arch_vm_translation_map_init: no memory for mappings pool pgtables.");
    /* init tables with zeroes */
    memset(map_pool_pgtables, MAP_POOL_PGTABLE_CHUNKS * PAGE_SIZE, 0);

    /* now put mappings pool page tables directly into the kernel page directory
     * these will be wired and kept mapped into virtual space to be easy to get to
     * by map_pool_chunk function called from page mapper.
     */
    {
        uint i, base_ent = map_pool_base / MAP_POOL_PGTABLE_CHUNK_SIZE;
        addr_t phys_pgtable;
        addr_t virt_pgtable;
        mmu_pde *ent;

        virt_pgtable = (addr_t)map_pool_pgtables;
        for(i = 0; i < MAP_POOL_PGTABLE_CHUNKS; i++, virt_pgtable += PAGE_SIZE) {
            vm_tmap_quick_query(virt_pgtable, &phys_pgtable);
            ent = &page_hole_pgdir[base_ent + i];
            put_pgtable_in_pgdir(ent, phys_pgtable, VM_PROT_KERNEL_DEFAULT);
        }
    } /* end of page mapper init */


    /* and now, set global bit to all pages allocated in kernel space */
    {
      /*
       * Important note: This chunk of code assumes kernel base can be not PDE
       * aligned. So... If kernel base is not PDE aliged this unmaps all pages below
       * kernel base and set all needed flags correctly. But in practice kernel
       * base must be PDE aligned or this module will not work.
       */
      uint kbase_pde = VADDR_TO_PDENT(KERNEL_BASE);  /* kernel base pd entry */
      uint kbase_pte = VADDR_TO_PTENT(KERNEL_BASE);  /* kernel base pt entry */
      mmu_pte *pgtable;   /* page table */
      uint i, j, first_pgent;

      /* walk through page directory entries */
      for(i=kbase_pde; i<MAX_PDENTS; i++) {
         /* ignore recursive page directory entry */
         if(i==page_hole_pdeidx) continue;
         /* if page table exists */
         if(page_hole_pgdir[i].stru.p) {
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
             pgtable = (mmu_pte *)((uint32)page_hole + i * PAGE_SIZE);
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
      /* also we need unmap everything below kernel space
       * so.... do it in the same manner
       */
      for(i=0; i<=kbase_pde; i++) {
         if(i==page_hole_pdeidx) continue;

         /* if page table exists */
         if(page_hole_pgdir[i].stru.p) {
             if(i==kbase_pde) {
               /* unmap pages below kernel space */
               pgtable = (mmu_pte *)((uint32)page_hole + i * PAGE_SIZE);
               for(j=0; j<kbase_pte; j++)
                   pgtable[j].stru.p = 0;
             } else {
               /* unmap bage table */
               page_hole_pgdir[i].stru.p = 0;
             }
         }
      }
      /* Turn on Global bit if supported. This prevents kernel pages
       * from being flushed from TLB on context-switch.
       * Note that only bootstrap processor enters this section during
       * kernel initialization stage.
       */
      if(ProcessorSet.processors[BOOTSTRAP_CPU].arch.features[I386_FEATURE_D] & X86_CPUID_PGE) {
          uint32 cr4; /* temp storage for CR4 register */
          cr4 = read_cr4();
          write_cr4(cr4 | X86_CR4_PGE); /* Set PGE bit in CR4 */
      }
    } /* end of Set Global Bit block */

    return NO_ERROR;
}

/* prefinal initialization stage */
status_t vm_translation_map_init_prefinal(kernel_args_t *kargs)
{
    /* deallocate page hole */
    deallocate_page_hole(kernel_pgdir_virt, page_hole_pdeidx, true);
    page_hole        = NULL;
    page_hole_pgdir  = NULL;
    page_hole_pdeidx = (-1);

    return NO_ERROR;
}

/* final initialization stage */
status_t vm_translation_map_init_final(kernel_args_t *kargs)
{
    aspace_id kid = vm_get_kernel_aspace_id();
    object_id id;
    status_t err;

    /* create kernel's page directory object */
    id = vm_create_physmem_object(VM_NAME_I386_KERNEL_PAGE_DIR, (addr_t)kernel_pgdir_phys,
                                  PAGE_SIZE, VM_OBJECT_PROTECT_ALL);
    if(id == VM_INVALID_OBJECTID)
        panic("vm_translation_map_init_final: failed to create kernel_page_dir object!\n");

    /* ... and mapping */
    err = vm_map_object_exactly(kid, id, VM_PROT_KERNEL_ALL, (addr_t)kernel_pgdir_virt);
    if(err != NO_ERROR)
        panic("vm_translation_map_init_final: failed to create kernel_page_dir mapping!\n");

    /* and init page reference counter for it */
    err = vm_page_init_wire_counters((addr_t)kernel_pgdir_virt, PAGE_SIZE);
    if(err != NO_ERROR)
        panic("vm_translation_map_init_final: failed to init page reference counter for kernel_page_dir!\n");

    /* create mapping pool page tables object and its mapping */
    id = vm_create_virtmem_object(VM_NAME_I386_KERNEL_MAP_POOL_PGTABLES, kid, (addr_t)map_pool_pgtables,
                                  MAP_POOL_PGTABLE_CHUNKS * PAGE_SIZE, VM_OBJECT_PROTECT_ALL);
    if(id == VM_INVALID_OBJECTID)
        panic("vm_translation_map_init_final: failed to create kernel_map_pool_pgtables object!\n");

    err = vm_map_object_exactly(kid, id, VM_PROT_KERNEL_ALL, (addr_t)map_pool_pgtables);
    if(err != NO_ERROR)
        panic("vm_translation_map_init_final: failed to create kernel_map_pool_pgtables mapping!\n");

    /* and init reference counters */
    err = vm_page_init_wire_counters((addr_t)map_pool_pgtables, MAP_POOL_PGTABLE_CHUNKS * PAGE_SIZE);
    if(err != NO_ERROR)
        panic("vm_translation_map_init_final: failed to init page reference counter for kernel_map_pool_pgtables!\n");

    /* now set wired state to all additional kernel's page tabel from kargs */
    {
        uint i;
        vm_page_t *page;

        for(i = 0; i < kargs->arch_args.num_pgtables; i++) {
            page = vm_page_lookup(PAGE_NUMBER(kargs->arch_args.phys_pgtables[i]));
            if(page == NULL)
                panic("vm_translation_map_init_final: failed to get page!\n");
            vm_page_set_state(page, VM_PAGE_STATE_WIRED);
        }
    }
    /* end */

    /* call final init stage of page mapper */
    err = vm_page_mapper_init_final(kargs);
    if(err != NO_ERROR)
        panic("vm_translation_map_init_final: page mapper final stage failed!\n");

    return NO_ERROR;
}

/*
 * Map a page directly without using any of Virtual Memory Manager objects.
 * Uses a 'page hole' also known as 'recursive directory entry'. The page hole is created by pointing
 * one of the page directory entries back at itself, effectively mapping the contents of all of the
 * 4Mb of page tables into a 4Mb region.
 * This routine used only during system start up. Do not use after.
 */
status_t vm_tmap_quick_map_page(kernel_args_t *kargs, addr_t virt_addr, addr_t phys_addr, uint protection)
{
    mmu_pte *pgentry;
    uint pgtable_idx;

    /* ensure page hole allocated */
    if(!page_hole)
        panic("vm_tmap_quick_map: page hole is not allocated. :(");

    /* check to see if a page table exists for this range */
    pgtable_idx = VADDR_TO_PDENT(virt_addr);
    if(!page_hole_pgdir[pgtable_idx].stru.p) {
        addr_t pgtable;
        mmu_pde *pdentry;
        /* we need to allocate a page table */
        pgtable = vm_alloc_ppage_from_kargs(kargs);
        if(!pgtable)
           panic("vm_tmap_quick_map: failed to allocate physical page.");
        /* page table is in pages, convert to physical address */
        pgtable *= PAGE_SIZE;

        /* put it in the pgdir */
        pdentry = &page_hole_pgdir[pgtable_idx];
        put_pgtable_in_pgdir(pdentry, pgtable, protection);

        /* zero it out in it's new mapping */
        memset((uint32 *)((uint32)page_hole + VADDR_TO_PDENT(virt_addr) * PAGE_SIZE), 0, PAGE_SIZE);
    }
    /* now, fill in the page entry */
    pgentry = page_hole + virt_addr / PAGE_SIZE;
    init_ptentry(pgentry);
    /* map physical address to virtual */
    pgentry->stru.base = ADDR_SHIFT(phys_addr);
    /* set page protection attributes */
    pgentry->stru.us = !(protection & VM_PROT_KERNEL) != 0;
    pgentry->stru.rw = (protection & VM_PROT_WRITE) != 0;
    pgentry->stru.p  = 1;
    if(is_kernel_address(virt_addr))
        pgentry->stru.g = 1; /* global bit set for all kernel addresses */

    /* invalidate newly mapped address */
    invalidate_TLB_entry(virt_addr);

    return NO_ERROR;
}

/*
 * Returns physical address of a page. Uses a 'page hole' hack.
 * This routine used only during system start up. Do not use after.
 */
status_t vm_tmap_quick_query(addr_t vaddr, addr_t *out_paddr)
{
    mmu_pte *pt_entry;

    /* ensure page hole allocated */
    if(!page_hole)
        panic("vm_tmap_quick_query: page hole is not allocated. :(");

    /* check that page table exists here */
    if(page_hole_pgdir[VADDR_TO_PDENT(vaddr)].stru.p == 0) {
        /* no pagetable here */
        return ERR_VM_PAGE_NOT_PRESENT;
    }

    /* get page table entry for this address */
    pt_entry = page_hole + vaddr / PAGE_SIZE;
    if(pt_entry->stru.p == 0) {
        /* page mapping not valid */
        return ERR_VM_PAGE_NOT_PRESENT;
    }

    /* store result */
    *out_paddr = ADDR_REVERSE_SHIFT(pt_entry->stru.base);

    return NO_ERROR;
}

/* Create translation map for kernel address space */
status_t vm_tmap_kernel_create(vm_translation_map_t *kernel_tmap)
{
    if(kernel_tmap == NULL)
        return ERR_INVALID_ARGS;

    /* call with known kernel page directory mapping */
    return vm_tmap_common_create(kernel_tmap, kernel_pgdir_virt, kernel_pgdir_phys);
}

/* Create translation map for user-space */
status_t vm_tmap_create(vm_translation_map_t *new_tmap)
{
    mmu_pde *pgdir_virt;
    mmu_pde *pgdir_phys;
    vm_address_space_t *kaspace;
    status_t err;

    /* check input arguments */
    if(new_tmap == NULL)
        return ERR_INVALID_ARGS;

    /* allocate a page for Page Directory */
    pgdir_virt = (mmu_pde *)kmalloc_pages(1);
    if(!pgdir_virt)
        return ERR_NO_MEMORY;

    /* get kernel address space */
    kaspace = vm_get_kernel_aspace();

    /* query physical address of allocated Page Directory */
    err = vm_query_paddr(kaspace, (addr_t)pgdir_virt, (addr_t*)&pgdir_phys, NULL);
    if(err != NO_ERROR)
        goto error_exit;

    /* create new address space */
    err = vm_tmap_common_create(new_tmap, pgdir_virt, pgdir_phys);
    if(err != NO_ERROR)
        goto error_exit;

    /* return kernel space */
    vm_put_aspace(kaspace);

    return NO_ERROR;

error_exit:
    /* release resources on error */
    vm_put_aspace(kaspace);
    kfree(pgdir_virt);

    return err;
}
