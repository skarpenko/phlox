/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/kernel.h>
#include <phlox/list.h>
#include <phlox/arch/vm_transmap.h>
#include <phlox/processor.h>
#include <phlox/spinlock.h>
#include <phlox/errors.h>
#include <phlox/vm.h>
#include <phlox/vm_page.h>

/* type redefinition for convenience */
typedef xlist_t page_list_t;

/* Lists of pages */
static page_list_t  free_pages_list;    /* List of free pages   */
static page_list_t  active_pages_list;  /* List of active pages */

/* Array of all available pages */
static vm_page_t *all_pages;

/* spinlock for operations with pages */
static spinlock_t page_lock;

/* Offset of first available physical page */
static addr_t physical_page_offset;
/* Total number of pages */
static uint32 total_pages_count;


/**
 ** Locally used operations with page lists.
 ** (Note: No locks used!)
 **/
/* put page to end of the list */
static void put_page_to_list(page_list_t *list, vm_page_t *page) {
    xlist_add_last(list, &page->list_node);
}

/* extract first page from list */
static vm_page_t *get_page_from_list(page_list_t *list) {
    list_elem_t *tmp = xlist_extract_first(list);
    return containerof(tmp, vm_page_t, list_node);
}

/* remove page from list */
static void remove_page_from_list(page_list_t *list, vm_page_t *page) {
    xlist_remove(list, &page->list_node);
}

/* move page from first list to end of second list */
static void move_page_to_list(page_list_t *list_from, page_list_t *list_to, vm_page_t *page) {
    if(list_from != list_to) {
        xlist_remove(list_from, &page->list_node);
        xlist_add_last(list_to, &page->list_node);
    }
}

/**
 ** Locally used operations with pages.
 ** (Note: No locks used!)
 **/
static uint32 set_page_state(vm_page_t *page, uint32 page_state) {
    page_list_t *from_list = NULL;
    page_list_t *to_list   = NULL;

    /* if page state and requested state is equal */
    if(page->state == page_state)
      return 0;

    /* get source page list */
    switch(page->state) {
        case VM_PAGE_STATE_ACTIVE:
           VM_State.active_pages--;
           from_list = &active_pages_list;
           break;
        case VM_PAGE_STATE_FREE:
           VM_State.free_pages--;
           from_list = &free_pages_list;
           break;
        case VM_PAGE_STATE_UNUSED:
           VM_State.unused_pages--;
           from_list = &active_pages_list;
           break;
        default:
            panic("vm_page_set_state: vm_page %p in invalid state %d\n", page, page->state);
    }

    /* get destination page list */
    switch(page_state) {
        case VM_PAGE_STATE_ACTIVE:
           VM_State.active_pages++;
           to_list = &active_pages_list;
           break;
        case VM_PAGE_STATE_FREE:
           VM_State.free_pages++;
           to_list = &free_pages_list;
           break;
        case VM_PAGE_STATE_UNUSED:
           VM_State.unused_pages++;
           to_list = &active_pages_list;
           break;
        default:
            panic("vm_page_set_state: invalid target state %d\n", page_state);
    }

    /* move page to new list */
    move_page_to_list(from_list, to_list, page);

    /* set new state */
    page->state = page_state;

    return 0;
}


/* pre initialization routine */
uint32 vm_page_preinit(kernel_args_t *kargs) {
    uint32 i, last_phys_page = 0;

    /* init lists */
    xlist_init(&free_pages_list);
    xlist_init(&active_pages_list);

    /* init spinlock */
    spin_init(&page_lock);

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
    uint32 i;

    /* allocate area for pages structures */
    all_pages = (vm_page_t *)vm_alloc_from_kargs(kargs, total_pages_count*sizeof(vm_page_t),
                                                   VM_LOCK_KERNEL|VM_LOCK_RW);
    /* init pages */
    for(i = 0; i < total_pages_count; i++) {
       all_pages[i].ppn       = physical_page_offset;
       all_pages[i].type      = VM_PAGE_TYPE_PHYS;
       all_pages[i].state     = VM_PAGE_STATE_FREE;
       all_pages[i].ref_count = 0;
       VM_State.free_pages++;
       put_page_to_list(&free_pages_list, &all_pages[i]);
    }

    /* mark some physically allocated ranges of pages as used */
    for(i = 0; i < kargs->num_phys_alloc_ranges; i++) {
        vm_page_mark_range_inuse(kargs->phys_alloc_range[i].start / PAGE_SIZE,
                                 kargs->phys_alloc_range[i].size / PAGE_SIZE);
    }

    return 0;
}

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

/* mark physical page as used */
uint32 vm_page_mark_page_inuse(addr_t page) {
   return vm_page_mark_range_inuse(page, 1);
}

/* mark range of physical pages as used */
uint32 vm_page_mark_range_inuse(addr_t start_page, size_t len_pages) {
    unsigned long irqs_state;
    vm_page_t *page;
    addr_t i;

    /* check arguments */
    if(physical_page_offset > start_page) {
        kprint("vm_page_mark_range_inuse: start page %ld is before free list\n", start_page);
        return ERR_INVALID_ARGS;
    }
    start_page -= physical_page_offset;
    if(start_page + len_pages >= total_pages_count) {
        kprint("vm_page_mark_range_inuse: range would extend past free list\n");
        return ERR_INVALID_ARGS;
    }

    /* acquire lock */
    irqs_state = spin_lock_irqsave(&page_lock);

    /* process pages */
    for(i = 0; i < len_pages; i++) {
       page = &all_pages[start_page + i];
       switch(page->state) {
         case VM_PAGE_STATE_FREE:
           set_page_state(page, VM_PAGE_STATE_UNUSED);
           break;
         case VM_PAGE_STATE_ACTIVE:
         case VM_PAGE_STATE_UNUSED:
         default:
           kprint("vm_page_mark_range_inuse: page 0x%lx in non-free state %d!\n",
                        start_page + i, page->state);
       }
    }

    /* release lock */
    spin_unlock_irqrstor(&page_lock, irqs_state);

    return NO_ERROR;
}
