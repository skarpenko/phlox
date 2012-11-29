/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/kernel.h>

/* allocate virtual space from kernel args */
static addr_t vm_alloc_vspace_from_kargs(uint32 size) {
    addr_t spot = 0;
    uint32 i;
    int last_valloc_entry = 0;

    size = PAGE_ALIGN(size);
    
    /* find a slot in the virtual allocation addr_t range */
    for(i=1; i < globalKargs.num_virt_alloc_ranges; i++) {
        last_valloc_entry = i;
        /* check to see if the space between this one and the last is big enough */
        if(globalKargs.virt_alloc_range[i].start -
          (globalKargs.virt_alloc_range[i-1].start +
           globalKargs.virt_alloc_range[i-1].size) >= size) {
             spot = globalKargs.virt_alloc_range[i-1].start +
                     globalKargs.virt_alloc_range[i-1].size;
             globalKargs.virt_alloc_range[i-1].size += size;
             goto out;
        }
    }
    if(spot == 0) {
        /* we hadn't found one between allocation ranges. this is ok.
         * see if there's a gap after the last one
         */
        if(globalKargs.virt_alloc_range[last_valloc_entry].start +
           globalKargs.virt_alloc_range[last_valloc_entry].size + size <=
              KERNEL_BASE + (KERNEL_SIZE - 1)) {
              spot = globalKargs.virt_alloc_range[last_valloc_entry].start +
                      globalKargs.virt_alloc_range[last_valloc_entry].size;
              globalKargs.virt_alloc_range[last_valloc_entry].size += size;
              goto out;
        }
        /* see if there's a gap before the first one */
        if(globalKargs.virt_alloc_range[0].start > KERNEL_BASE) {
            if(globalKargs.virt_alloc_range[0].start - KERNEL_BASE >= size) {
                globalKargs.virt_alloc_range[0].start -= size;
                spot = globalKargs.virt_alloc_range[0].start;
                goto out;
            }
        }
    }

    out: return spot;
}

/* horrible brute-force method of determining if the page can be allocated */
static uint32 is_page_in_phys_range(addr_t paddr) {
    uint32 i;

    for(i=0; i < globalKargs.num_phys_mem_ranges; i++) {
        /* if physical page lies in one of availabe physical ranges
         * it can be allocated.
         */
        if(paddr >= globalKargs.phys_mem_range[i].start &&
           paddr <  globalKargs.phys_mem_range[i].start +
           globalKargs.phys_mem_range[i].size) {
              return 1;
        }
    }
    /* page cannot be allocated */
    return 0;
}

/* allocate physical page from kernel args */
static addr_t vm_alloc_phpage_from_kargs() {
    uint32 i;
    addr_t next_page;

    for(i=0; i < globalKargs.num_phys_alloc_ranges; i++) {
        next_page = globalKargs.phys_alloc_range[i].start +
                     globalKargs.phys_alloc_range[i].size;
        /* see if the page after the next allocated paddr run can be allocated */
        if(i + 1 < globalKargs.num_phys_alloc_ranges &&
           globalKargs.phys_alloc_range[i+1].size != 0) {
            /* see if the next page will collide with the next allocated range */
            if(next_page >= globalKargs.phys_alloc_range[i+1].start)
                continue;
        }
        /* see if the next physical page fits in the memory block */
        if(is_page_in_phys_range(next_page)) {
            /* we got one! */
            globalKargs.phys_alloc_range[i].size += PAGE_SIZE;
            return ( (globalKargs.phys_alloc_range[i].start +
                       globalKargs.phys_alloc_range[i].size - PAGE_SIZE) / PAGE_SIZE);
        }
    }
    /* could not allocate a block :( */
    return 0;
}

addr_t vm_alloc_from_kargs(uint32 size, uint32 lock) {
    addr_t vspot;
    addr_t pspot;
    uint32 i;

    /* find the vaddr to allocate at */
    vspot = vm_alloc_vspace_from_kargs(size);

    /* map the pages */
    for(i=0; i < PAGE_ALIGN(size)/PAGE_SIZE; i++) {
        pspot = vm_alloc_phpage_from_kargs();
        if(pspot == 0)
            panic("error allocating physical page from globalKargs!\n");
/**        vm_translation_map_quick_map(vspot + i*PAGE_SIZE, pspot * PAGE_SIZE, lock, &vm_alloc_ppage_from_kernel_struct); */
    }

    return vspot;
}
