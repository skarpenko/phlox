/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Copyright 2001-2004, Travis Geiselbrecht. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <phlox/kernel.h>
#include <phlox/list.h>
#include <phlox/arch/vm_translation_map.h>
#include <phlox/processor.h>
#include <phlox/atomic.h>
#include <phlox/spinlock.h>
#include <phlox/errors.h>
#include <phlox/vm_private.h>
#include <phlox/vm.h>
#include <phlox/vm_page_mapper.h>
#include <phlox/vm_page.h>

/* type redefinition for convenience */
typedef xlist_t page_list_t;

/* Lists of pages */
static page_list_t  free_pages_list;    /* List of free pages   */
static page_list_t  clear_pages_list;   /* List of clear pages  */
static page_list_t  active_pages_list;  /* List of active pages */

/* Array of all available pages */
static vm_page_t *all_pages;

/* spinlock for operations with pages */
static spinlock_t page_lock;

/* Offset of first available physical page */
static addr_t physical_page_offset;
/* Total number of pages */
static uint total_pages_count;


/**
 ** Locally used operations with page lists.
 ** (Note: No locks used!)
 **/
/* put page to end of the list */
static void put_page_to_list(page_list_t *list, vm_page_t *page)
{
    xlist_add_last(list, &page->list_node);
}

/* extract first page from list */
static vm_page_t *get_page_from_list(page_list_t *list)
{
    list_elem_t *tmp = xlist_extract_first(list);
    return containerof(tmp, vm_page_t, list_node);
}

/* remove page from list */
static void remove_page_from_list(page_list_t *list, vm_page_t *page)
{
    xlist_remove(list, &page->list_node);
}

/* move page from first list to end of second list */
static void move_page_to_list(page_list_t *list_from, page_list_t *list_to, vm_page_t *page)
{
    if(list_from != list_to) {
        xlist_remove(list_from, &page->list_node);
        xlist_add_last(list_to, &page->list_node);
    }
}

/**
 ** Locally used operations with pages.
 ** (Note: No locks used!)
 **/
static uint set_page_state(vm_page_t *page, uint page_state)
{
    page_list_t *from_list = NULL;
    page_list_t *to_list   = NULL;

    /* if page state and requested state is equal */
    if(page->state == page_state)
      return 0;

    /* get source page list */
    switch(page->state) {
        /* pages from active_pages_list */
        case VM_PAGE_STATE_ACTIVE:
           VM_State.active_pages--;
           from_list = &active_pages_list;
           break;
        case VM_PAGE_STATE_INACTIVE:
           VM_State.inactive_pages--;
           from_list = &active_pages_list;
           break;
        case VM_PAGE_STATE_UNUSED:
           VM_State.unused_pages--;
           from_list = &active_pages_list;
           break;
        case VM_PAGE_STATE_WIRED:
           VM_State.wired_pages--;
           from_list = &active_pages_list;
           break;
        case VM_PAGE_STATE_BUSY:
           VM_State.busy_pages--;
           from_list = &active_pages_list;
           break;
        /* pages from other page lists */
        case VM_PAGE_STATE_FREE:
           VM_State.free_pages--;
           from_list = &free_pages_list;
           break;
        case VM_PAGE_STATE_CLEAR:
           VM_State.clear_pages--;
           from_list = &clear_pages_list;
           break;
        default:
            panic("set_page_state: vm_page %p in invalid state %d\n", page, page->state);
    }

    /* get destination page list */
    switch(page_state) {
        /* pages to active_pages_list */
        case VM_PAGE_STATE_ACTIVE:
           VM_State.active_pages++;
           to_list = &active_pages_list;
           break;
        case VM_PAGE_STATE_INACTIVE:
           VM_State.inactive_pages++;
           to_list = &active_pages_list;
           break;
        case VM_PAGE_STATE_UNUSED:
           VM_State.unused_pages++;
           to_list = &active_pages_list;
           break;
        case VM_PAGE_STATE_WIRED:
           VM_State.wired_pages++;
           to_list = &active_pages_list;
           break;
        case VM_PAGE_STATE_BUSY:
           VM_State.busy_pages++;
           to_list = &active_pages_list;
           break;
        /* pages to other page lists */
        case VM_PAGE_STATE_FREE:
           VM_State.free_pages++;
           to_list = &free_pages_list;
           break;
        case VM_PAGE_STATE_CLEAR:
           VM_State.clear_pages++;
           to_list = &clear_pages_list;
           break;
        default:
            panic("set_page_state: invalid target state %d\n", page_state);
    }

    /* move page to new list */
    move_page_to_list(from_list, to_list, page);

    /* set new state */
    page->state = page_state;

    return 0;
}

/* fill with zeroes given physical page */
static void clear_page(addr_t pa)
{
    addr_t va;

    /* ask mapper to map page */
    vm_pmap_get_ppage(pa, &va, true);
    /* clear it */
    memset((void *)va, 0, PAGE_SIZE);
    /* and tell mapper that we are done */
    vm_pmap_put_ppage(va);
}


/* pre initialization routine */
status_t vm_page_preinit(kernel_args_t *kargs)
{
    uint i, last_phys_page = 0;

    /* init lists */
    xlist_init(&free_pages_list);
    xlist_init(&clear_pages_list);
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
status_t vm_page_init(kernel_args_t *kargs)
{
    uint i;

    /* allocate area for pages structures */
    all_pages = (vm_page_t *)vm_alloc_from_kargs(kargs, total_pages_count*sizeof(vm_page_t),
                                                VM_PROT_KERNEL_DEFAULT);

    /* init pages */
    for(i = 0; i < total_pages_count; i++) {
       all_pages[i].ppn        = physical_page_offset + i;
       all_pages[i].type       = VM_PAGE_TYPE_PHYSICAL;
       all_pages[i].state      = VM_PAGE_STATE_FREE;
       all_pages[i].wire_count = 0;
       VM_State.free_pages++;
       put_page_to_list(&free_pages_list, &all_pages[i]);
    }

    /* mark some physically allocated ranges of pages as used */
    for(i = 0; i < kargs->num_phys_alloc_ranges; i++) {
        vm_page_mark_range_inuse(kargs->phys_alloc_range[i].start / PAGE_SIZE,
                                 kargs->phys_alloc_range[i].size / PAGE_SIZE);
    }

    return NO_ERROR;
}

/* final stage of module initialization */
status_t vm_page_init_final(kernel_args_t *kargs)
{
    aspace_id kid = vm_get_kernel_aspace_id();
    object_id id;
    status_t err;

    /* create object of pages array ... */
    id = vm_create_virtmem_object("kernel_physical_pages", kid, (addr_t)all_pages,
                                  total_pages_count * sizeof(vm_page_t),
                                  VM_OBJECT_PROTECT_ALL);
    if(id == VM_INVALID_OBJECTID)
        return ERR_GENERAL;

    /* ... and its mapping */
    err = vm_map_object_exactly(kid, id, VM_PROT_KERNEL_ALL, (addr_t)all_pages);
    if(err != NO_ERROR)
        return err;

    /* init page reference counters */
    err = vm_page_init_wire_counters((addr_t)all_pages, total_pages_count * sizeof(vm_page_t));

    return err;
}

/* horrible brute-force method of determining if the page can be allocated */
static uint is_page_in_phys_range(kernel_args_t *kargs, addr_t paddr)
{
    uint i;

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
addr_t vm_alloc_ppage_from_kargs(kernel_args_t *kargs)
{
    uint i;
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

/* init reference counters of physical pages */
status_t vm_page_init_wire_counters(addr_t vbase, size_t size)
{
    vm_address_space_t *aspace = vm_get_kernel_aspace();
    addr_t vaddr, paddr, end_addr;
    uint flags;
    vm_page_t *page;
    status_t err = NO_ERROR;

    /* init variables */
    vbase = ROUNDOWN(vbase, PAGE_SIZE);
    size = PAGE_ALIGN(size);
    end_addr = vbase + size - 1;

    /* now query all physical pages and set wired counter to 1 */
    for(vaddr = vbase; vaddr < end_addr; vaddr += PAGE_SIZE) {
        /* query physical page */
        (*aspace->tmap.ops->query)(&aspace->tmap, vaddr, &paddr, &flags);

        /* page must present in address space */
        if( !(flags & VM_FLAG_PAGE_PRESENT) ) {
            err = ERR_VM_PAGE_NOT_PRESENT;
            goto exit_init;
        }

        /* get page */
        page = vm_page_lookup(PAGE_NUMBER(paddr));
        if(page == NULL) {
            err = ERR_VM_GENERAL;
            goto exit_init;
        }

        /* atomically set value */
        atomic_set(&page->wire_count, 1);
    }

exit_init:
    /* put address space back and exit */
    vm_put_aspace(aspace);

    return err;
}

/* mark physical page as used */
status_t vm_page_mark_page_inuse(addr_t page)
{
   return vm_page_mark_range_inuse(page, 1);
}

/* mark range of physical pages as used */
status_t vm_page_mark_range_inuse(addr_t start_page, size_t len_pages)
{
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
         case VM_PAGE_STATE_CLEAR:
         case VM_PAGE_STATE_FREE:
           set_page_state(page, VM_PAGE_STATE_UNUSED);
           break;
         case VM_PAGE_STATE_WIRED:
           break;
         case VM_PAGE_STATE_ACTIVE:
         case VM_PAGE_STATE_INACTIVE:
         case VM_PAGE_STATE_BUSY:
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

/* allocate specific page by its number and state */
vm_page_t *vm_page_alloc_specific(addr_t page_num, uint page_state)
{
    vm_page_t *p;
    uint irqs_state;
    /* preset old page state. it needs on exit. */
    uint old_page_state = VM_PAGE_STATE_BUSY;

    /* acquire lock */
    irqs_state = spin_lock_irqsave(&page_lock);

    /* get asked page */
    p = vm_page_lookup(page_num);
    if(p == NULL)
        goto exit_allocate;

    /* remove page from proper list */
    switch(p->state) {
        case VM_PAGE_STATE_FREE:
            remove_page_from_list(&free_pages_list, p);
            VM_State.free_pages--;
            break;
        case VM_PAGE_STATE_CLEAR:
            remove_page_from_list(&clear_pages_list, p);
            VM_State.clear_pages--;
            break;
        case VM_PAGE_STATE_UNUSED:
            VM_State.unused_pages--;
            break;
        default:
            /* we can't allocate this page */
            p = NULL;
    }
    if(p == NULL)
        goto exit_allocate;

    /* store previous and set new page state */
    old_page_state = p->state;
    p->state = VM_PAGE_STATE_BUSY;
    VM_State.busy_pages++; /* statistics */

    if(old_page_state != VM_PAGE_STATE_UNUSED)
        put_page_to_list(&active_pages_list, p);

    /* complete our job */
exit_allocate:
    /* release lock */
    spin_unlock_irqrstor(&page_lock, irqs_state);

    if(p != NULL && page_state == VM_PAGE_STATE_CLEAR &&
        (old_page_state == VM_PAGE_STATE_FREE ||
         old_page_state == VM_PAGE_STATE_UNUSED)) {

        /* clear page if it is not allocated from
         * list of clear pages
         */
        clear_page(p->ppn * PAGE_SIZE);
    }

    return p;
}

/* allocate specific range of pages */
vm_page_t *vm_page_alloc_specific_range(addr_t first_page_num, addr_t npages, uint page_state)
{
    vm_page_t *p;
    vm_page_t *first_page = NULL;
    uint irqs_state;
    bool need_clear = false;
    uint old_page_state;
    addr_t i;

    if(npages == 0)
        return NULL;

    /* acquire lock */
    irqs_state = spin_lock_irqsave(&page_lock);

    /* check that allocation is possible */
    for(i = first_page_num; i < first_page_num + npages; i++) {
        /* get page */
        p = vm_page_lookup(i);
        if(p == NULL || (p->state != VM_PAGE_STATE_FREE  &&
                         p->state != VM_PAGE_STATE_CLEAR &&
                         p->state != VM_PAGE_STATE_UNUSED))
            goto exit_allocate;
    }

    /* adjust index for direct access to pages array */
    first_page_num -= physical_page_offset;

    /* now we can allocate pages */
    for(i = first_page_num; i < first_page_num + npages; i++) {
        p = &all_pages[i];
        /* remove page from proper list */
        switch(p->state) {
            case VM_PAGE_STATE_FREE:
                remove_page_from_list(&free_pages_list, p);
                VM_State.free_pages--;
                if(page_state == VM_PAGE_STATE_CLEAR)
                    need_clear = true;
                break;
            case VM_PAGE_STATE_CLEAR:
                remove_page_from_list(&clear_pages_list, p);
                VM_State.clear_pages--;
                break;
            case VM_PAGE_STATE_UNUSED:
                VM_State.unused_pages--;
                if(page_state == VM_PAGE_STATE_CLEAR)
                    need_clear = true;
                break;
            default:
                panic("vm_page_alloc_specific_range(): page changed its state!");
        }

        /* set new state and store old one */
        old_page_state = p->state;
        p->state = VM_PAGE_STATE_BUSY;

        VM_State.busy_pages++; /* update statistics */

        /* put to active pages list if page is not in it */
        if(old_page_state != VM_PAGE_STATE_UNUSED)
            put_page_to_list(&active_pages_list, p);
    }

    /* store first page of the range for caller */
    first_page = &all_pages[first_page_num];

exit_allocate:
    /* release lock */
    spin_unlock_irqrstor(&page_lock, irqs_state);

    /* clear pages if needed */
    if(need_clear) {
        for(i = first_page_num; i < first_page_num + npages; i++)
            clear_page(all_pages[i].ppn * PAGE_SIZE);
    }

    return first_page;
}

/* allocate page by state */
vm_page_t *vm_page_alloc(uint page_state)
{
    vm_page_t *p;
    unsigned long irqs_state;
    page_list_t *pages_list;
    page_list_t *spare_pages_list;
    uint old_page_state;

    /* set source list and spare list.
     * we can allocate only from list of free pages
     * or clear pages.
     */
    switch(page_state) {
        case VM_PAGE_STATE_FREE:
            pages_list = &free_pages_list;
            spare_pages_list = &clear_pages_list;
            break;
        case VM_PAGE_STATE_CLEAR:
            pages_list = &clear_pages_list;
            spare_pages_list = &free_pages_list;
            break;
        default:
            return NULL; /* invalid page state */
    }

    /* acquire lock */
    irqs_state = spin_lock_irqsave(&page_lock);

    p = get_page_from_list(pages_list);
    if(p == NULL) {
        /* pages list is empty. use spare pages list */
        pages_list = spare_pages_list;
        p = get_page_from_list(pages_list);
        if(p == NULL) {
            /* panic. spare pages list is empty too! */
            panic("vm_page_alloc: out of memory!\n");
        }
    }

    /* update statistics */
    if(pages_list == &free_pages_list)
        VM_State.free_pages--;
    else
        VM_State.clear_pages--;

    old_page_state = p->state;
    p->state = VM_PAGE_STATE_BUSY;
    VM_State.busy_pages++;

    /* put page to new list */
    put_page_to_list(&active_pages_list, p);

    /* release lock */
    spin_unlock_irqrstor(&page_lock, irqs_state);

    if(page_state == VM_PAGE_STATE_CLEAR &&
       old_page_state == VM_PAGE_STATE_FREE) {
        /* clear page if needed */
        clear_page(p->ppn * PAGE_SIZE);
    }

    return p;
}

/* allocate range of pages */
vm_page_t *vm_page_alloc_range(uint page_state, addr_t npages)
{
    unsigned long irqs_state;
    uint start;
    uint i;
    bool foundit;
    vm_page_t *first_page = NULL;

    start = 0;

    /* acquire lock */
    irqs_state = spin_lock_irqsave(&page_lock);

    for(;;) {
        foundit = true;
        if(start + npages >= total_pages_count)
            break;
        /* locate continuous chunk of pages */
        for(i = 0; i < npages; i++) {
            if(all_pages[start + i].state != VM_PAGE_STATE_FREE &&
              all_pages[start + i].state != VM_PAGE_STATE_CLEAR) {
                foundit = false;
                i++;
                break;
            }
        }
        if(foundit) {
            /* pull the pages out of the appropriate lists */
            for(i = 0; i < npages; i++)
                set_page_state(&all_pages[start + i], VM_PAGE_STATE_BUSY);
            first_page = &all_pages[start];
            break;
        } else {
            start += i;
            if(start >= total_pages_count) {
                /* no more pages to look through */
                break;
            }
        }
    }
    /* release lock */
    spin_unlock_irqrstor(&page_lock, irqs_state);

    return first_page;
}

/* look up a page */
vm_page_t *vm_page_lookup(addr_t page_num)
{
    /* ensure that page number correct */
    if(page_num < physical_page_offset)
        return NULL;
    page_num -= physical_page_offset;
    if(page_num >= total_pages_count)
        return NULL;

    /* return page to caller */
    return &all_pages[page_num];
}

/* set page state */
status_t vm_page_set_state(vm_page_t *page, uint page_state)
{
    status_t err;
    unsigned long irqs_state;

    /* acquire lock */
    irqs_state = spin_lock_irqsave(&page_lock);

    err = set_page_state(page, page_state);

    /* release lock */
    spin_unlock_irqrstor(&page_lock, irqs_state);

    return err;
}

/* return total pages count */
size_t vm_page_pages_count(void)
{
    return total_pages_count;
}

/* return free pages count*/
size_t vm_page_free_pages_count(void)
{
    /* return sum of pages from free and clear page lists */
    return (free_pages_list.count + clear_pages_list.count);
}
