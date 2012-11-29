/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/kernel.h>
#include <phlox/list.h>
#include <phlox/arch/vm_transmap.h>
#include <phlox/vm.h>
#include <phlox/vm_page.h>

/* Lists of pages */
static xlist_t  free_pages;    /* List of free pages   */
static xlist_t  active_pages;  /* List of active pages */

/* Array of all available pages */
static vm_page_t *all_pages;
/* Offset of first available physical page */
static addr_t physical_page_offset;
/* Total number of pages */
static uint32 total_pages_count;


/* allocate virtual space from kernel args */
addr_t vm_alloc_vspace_from_kargs(kernel_args_t *kargs, uint32 size) {
    addr_t spot = 0;
    uint32 i;
    int last_valloc_entry = 0;

    size = PAGE_ALIGN(size);
    
    /* find a slot in the virtual allocation addr_t range */
    for(i=1; i < kargs->num_virt_alloc_ranges; i++) {
        last_valloc_entry = i;
        /* check to see if the space between this one and the last is big enough */
        if(kargs->virt_alloc_range[i].start -
          (kargs->virt_alloc_range[i-1].start +
           kargs->virt_alloc_range[i-1].size) >= size) {
             spot = kargs->virt_alloc_range[i-1].start +
                     kargs->virt_alloc_range[i-1].size;
             kargs->virt_alloc_range[i-1].size += size;
             goto out;
        }
    }
    if(spot == 0) {
        /* we hadn't found one between allocation ranges. this is ok.
         * see if there's a gap after the last one
         */
        if(kargs->virt_alloc_range[last_valloc_entry].start +
           kargs->virt_alloc_range[last_valloc_entry].size + size <=
              KERNEL_BASE + (KERNEL_SIZE - 1)) {
              spot = kargs->virt_alloc_range[last_valloc_entry].start +
                      kargs->virt_alloc_range[last_valloc_entry].size;
              kargs->virt_alloc_range[last_valloc_entry].size += size;
              goto out;
        }
        /* see if there's a gap before the first one */
        if(kargs->virt_alloc_range[0].start > KERNEL_BASE) {
            if(kargs->virt_alloc_range[0].start - KERNEL_BASE >= size) {
                kargs->virt_alloc_range[0].start -= size;
                spot = kargs->virt_alloc_range[0].start;
                goto out;
            }
        }
    }

    out: return spot;
}

/* horrible brute-force method of determining if the page can be allocated */
static uint32 is_page_in_phys_range(kernel_args_t *kargs, addr_t paddr) {
    uint32 i;

    for(i=0; i < kargs->num_phys_mem_ranges; i++) {
        /* if physical page lies in one of availabe physical ranges
         * it can be allocated.
         */
        if(paddr >= kargs->phys_mem_range[i].start &&
           paddr <  kargs->phys_mem_range[i].start +
           kargs->phys_mem_range[i].size) {
              return 1;
        }
    }
    /* page cannot be allocated */
    return 0;
}

/* pre initialization routine */
uint32 vm_page_preinit(kernel_args_t *kargs) {
    uint32 i, last_phys_page = 0;

    /* init lists */
    xlist_init(&free_pages);
    xlist_init(&active_pages);

    /*** Calculate the size of memory by looking at the phys_mem_range array ***/
    /* first available physical page */
    physical_page_offset = kargs->phys_mem_range[0].start / PAGE_SIZE;
    /* search for last available physical page */
    for(i=0; i < kargs->num_phys_mem_ranges; i++) {
       last_phys_page = (kargs->phys_mem_range[i].start +
                         kargs->phys_mem_range[i].size) / PAGE_SIZE - 1;
    }
    /* number of available physical pages */
    total_pages_count = last_phys_page - physical_page_offset + 1;

    /* set up the global info structure about physical memory */
    VM_State.physical_page_size   = PAGE_SIZE;
    VM_State.total_physical_pages = total_pages_count;

    return 0;
}

/* module initialization routine */
uint32 vm_page_init(kernel_args_t *kargs) {
    /* do something */
    return 0;
}

/* allocate physical page from kernel args */
addr_t vm_alloc_phpage_from_kargs(kernel_args_t *kargs) {
    uint32 i;
    addr_t next_page;

    for(i=0; i < kargs->num_phys_alloc_ranges; i++) {
        next_page = kargs->phys_alloc_range[i].start +
                     kargs->phys_alloc_range[i].size;
        /* see if the page after the next allocated paddr run can be allocated */
        if(i + 1 < kargs->num_phys_alloc_ranges &&
           kargs->phys_alloc_range[i+1].size != 0) {
            /* see if the next page will collide with the next allocated range */
            if(next_page >= kargs->phys_alloc_range[i+1].start)
                continue;
        }
        /* see if the next physical page fits in the memory block */
        if(is_page_in_phys_range(kargs, next_page)) {
            /* we got one! */
            kargs->phys_alloc_range[i].size += PAGE_SIZE;
            return ( (kargs->phys_alloc_range[i].start +
                       kargs->phys_alloc_range[i].size - PAGE_SIZE) / PAGE_SIZE);
        }
    }
    /* could not allocate a block :( */
    return 0;
}

/* allocates memory block of given size form kernel args */
addr_t vm_alloc_from_kargs(kernel_args_t *kargs, uint32 size, uint32 attributes) {
    addr_t vspot;
    addr_t pspot;
    uint32 i;

    /* find the vaddr to allocate at */
    vspot = vm_alloc_vspace_from_kargs(kargs, size);

    /* map the pages */
    for(i=0; i < PAGE_ALIGN(size)/PAGE_SIZE; i++) {
        pspot = vm_alloc_phpage_from_kargs(kargs);
        if(pspot == 0)
            panic("error allocating physical page from globalKargs!\n");
        vm_transmap_quick_map_page(kargs, vspot + i*PAGE_SIZE, pspot * PAGE_SIZE, attributes);
    }

    /* return start address of allocated block */
    return vspot;
}
