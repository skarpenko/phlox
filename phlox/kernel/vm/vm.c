/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <phlox/errors.h>
#include <phlox/vm_page.h>
#include <phlox/vm.h>
#include <phlox/arch/vm_translation_map.h>
#include <phlox/heap.h>

/* Global variable with Virtual Memory State */
vm_stat_t VM_State;


/* init virtual memory */
status_t vm_init(kernel_args_t *kargs)
{
    status_t err;

    /* clear VM statistics */
    memset(&VM_State, 0, sizeof(vm_stat_t));

    /* translation map module init */
    err = vm_translation_map_init(kargs);
    if(err)
       panic("vm_init: translation map init failed!\n");

    /* execute architecture-specific init */
    err = arch_vm_init(kargs);
    if(err)
       panic("arch_vm_init: failed!\n");

    /* init memory size */
    err = vm_page_preinit(kargs);
    if(err)
       panic("vm_page_preinit: failed!\n");

    /* init kernel's heap */
    {
        addr_t heap_base;
        size_t heap_size;

        /* compute heap size */
        heap_size = ROUNDUP(
                      vm_phys_mem_size() / SYSCFG_KERNEL_HEAP_FRAC,
                      1*1024*1024 /* Mbyte */
                    );
        if(heap_size > SYSCFG_KERNEL_HEAP_MAX)
              heap_size = SYSCFG_KERNEL_HEAP_MAX;

        /* allocate heap area */
        heap_base = vm_alloc_from_kargs(kargs, heap_size, VM_LOCK_KERNEL | VM_LOCK_RW);
        /* init heap */
        heap_init(heap_base, heap_size);
        /* Fuf... Now kmalloc and kfree is available */
    }

    /* init vm page module */
    err = vm_page_init(kargs);
    if(err)
       panic("vm_page_init: failed!\n");

/*** Important note: After this point vm_alloc_from_kargs must not be used
 *** because physical pages bookkeping is turned on.
 ***/
 
    /* final stage of translation map module init */
    err = vm_translation_map_init_final(kargs);
    if(err)
       panic("vm_init: final stage of translation map init failed!\n");

    /* start final init stage of architecture-specific parts */
    err = arch_vm_init_final(kargs);
    if(err)
       panic("arch_vm_init_final: final stage failed!\n");

    return NO_ERROR;
}

/* allocate virtual space from kernel args */
addr_t vm_alloc_vspace_from_kargs(kernel_args_t *kargs, size_t size)
{
    addr_t spot = 0;
    uint i;
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

/* allocates memory block of given size form kernel args */
addr_t vm_alloc_from_kargs(kernel_args_t *kargs, size_t size, uint attributes)
{
    addr_t vspot;
    addr_t pspot;
    uint i;

    /* find the vaddr to allocate at */
    vspot = vm_alloc_vspace_from_kargs(kargs, size);

    /* map the pages */
    for(i=0; i < PAGE_ALIGN(size)/PAGE_SIZE; i++) {
        pspot = vm_alloc_ppage_from_kargs(kargs);
        if(pspot == 0)
            panic("error allocating physical page from globalKargs!\n");
        vm_tmap_quick_map_page(kargs, vspot + i*PAGE_SIZE, pspot * PAGE_SIZE, attributes);
    }

    /* return start address of allocated block */
    return vspot;
}

/* return available physical memory size */
size_t vm_phys_mem_size(void)
{
    return VM_State.total_physical_pages * VM_State.physical_page_size;
}
