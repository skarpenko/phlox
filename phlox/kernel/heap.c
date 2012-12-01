/*
* Copyright 2007-2011, Stepan V.Karpenko. All rights reserved.
* Copyright 2001, Travis Geiselbrecht. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/

/* Ported from NewOS.
 * Available chunks of memory are maintained in bins, grouped by size.
 * Allocator searches for available chunks are processed in smallest-first,
 * best-fit order. As shown by Wilson et al, best-fit schemes (of various kinds
 * and approximations) tend to produce the least fragmentation on real loads
 * compared to other general approaches such as first-fit.
 * Note: Bins for sizes less than PAGE_SIZE grows until page filled.
 */

#include <sys/debug.h>
#include <string.h>
#include <arch/cpu.h>
#include <phlox/errors.h>
#include <phlox/simple_lock.h>
#include <phlox/heap.h>


/*** Debug flags ***/
#define PARANOID_KFREE 1
#define WIPE_KFREE 1
#define MAKE_NOIZE 0

/* heap stuff
 * ripped mostly from nujeffos
 */

/* page info */
struct heap_page {
    uint16 bin_index  : 5;
    uint16 free_count : 9;
    uint16 cleaning   : 1;
    uint16 in_use     : 1;
};

/* heap allocation table. maintains free
 * space and free pages of the heap.
*/
static struct heap_page *heap_alloc_table;

static addr_t heap_base_ptr;
static addr_t heap_base;
static size_t heap_size;

/* bin */
struct heap_bin {
    uint32 element_size;
    uint32 grow_size;
    uint32 alloc_count;
    void   *free_list;
    uint32 free_count;
    char   *raw_list;
    uint32 raw_count;
};

/* bins list */
static struct heap_bin bins[] = {
 /*  0 */  { 16,      PAGE_SIZE, 0, 0, 0, 0, 0 },  /* 16b    */
 /*  1 */  { 32,      PAGE_SIZE, 0, 0, 0, 0, 0 },  /* 32b    */
 /*  2 */  { 64,      PAGE_SIZE, 0, 0, 0, 0, 0 },  /* 64b    */
 /*  3 */  { 128,     PAGE_SIZE, 0, 0, 0, 0, 0 },  /* 128b   */
 /*  4 */  { 256,     PAGE_SIZE, 0, 0, 0, 0, 0 },  /* 256b   */
 /*  5 */  { 512,     PAGE_SIZE, 0, 0, 0, 0, 0 },  /* 512b   */
 /*  6 */  { 1024,    PAGE_SIZE, 0, 0, 0, 0, 0 },  /* 1Kb    */
 /*  7 */  { 2048,    PAGE_SIZE, 0, 0, 0, 0, 0 },  /* 2Kb    */
 /*  8 */  { 0x1000,  0x1000,    0, 0, 0, 0, 0 },  /* 4Kb    */
 /*  9 */  { 0x2000,  0x2000,    0, 0, 0, 0, 0 },  /* 8Kb    */
 /* 10 */  { 0x3000,  0x3000,    0, 0, 0, 0, 0 },  /* 12Kb   */
 /* 11 */  { 0x4000,  0x4000,    0, 0, 0, 0, 0 },  /* 16Kb   */
 /* 12 */  { 0x5000,  0x5000,    0, 0, 0, 0, 0 },  /* 20Kb   */
 /* 13 */  { 0x6000,  0x6000,    0, 0, 0, 0, 0 },  /* 24Kb   */
 /* 14 */  { 0x7000,  0x7000,    0, 0, 0, 0, 0 },  /* 28Kb   */
 /* 15 */  { 0x8000,  0x8000,    0, 0, 0, 0, 0 },  /* 32Kb   */
 /* 16 */  { 0x9000,  0x9000,    0, 0, 0, 0, 0 },  /* 36Kb   */
 /* 17 */  { 0xa000,  0xa000,    0, 0, 0, 0, 0 },  /* 40Kb   */
 /* 18 */  { 0xb000,  0xb000,    0, 0, 0, 0, 0 },  /* 44Kb   */
 /* 19 */  { 0xc000,  0xc000,    0, 0, 0, 0, 0 },  /* 48Kb   */
 /* 20 */  { 0xd000,  0xd000,    0, 0, 0, 0, 0 },  /* 52Kb   */
 /* 21 */  { 0xe000,  0xe000,    0, 0, 0, 0, 0 },  /* 56Kb   */
 /* 22 */  { 0xf000,  0xf000,    0, 0, 0, 0, 0 },  /* 60Kb   */
 /* 23 */  { 0x10000, 0x10000,   0, 0, 0, 0, 0 },  /* 64Kb   */
 /* 24 */  { 0x11000, 0x11000,   0, 0, 0, 0, 0 },  /* 68Kb   */
 /* 25 */  { 0x12000, 0x12000,   0, 0, 0, 0, 0 },  /* 72Kb   */
 /* 26 */  { 0x13000, 0x13000,   0, 0, 0, 0, 0 },  /* 76Kb   */
 /* 27 */  { 0x14000, 0x14000,   0, 0, 0, 0, 0 },  /* 80Kb   */
 /* 28 */  { 0x15000, 0x15000,   0, 0, 0, 0, 0 },  /* 84Kb   */
 /* 29 */  { 0x16000, 0x16000,   0, 0, 0, 0, 0 },  /* 88Kb   */
 /* 30 */  { 0x17000, 0x17000,   0, 0, 0, 0, 0 },  /* 92Kb   */
 /* 31 */  { 0x18000, 0x18000,   0, 0, 0, 0, 0 },  /* 96Kb   */
/*
 * Note: Bin_Index = 31 is maximum possible, 'cause only 5 bits
 * used for it in heap_page structure.
 */
};

static const int bin_count = sizeof(bins) / sizeof(struct heap_bin);
static simple_lock_t heap_lock;
static bool threading = false;

#define HEAP_LOCK(irq_state) \
    if(threading) \
        simple_lock(&heap_lock); \
    else \
        irq_state = spin_lock_irqsave(&heap_lock)
        

#define HEAP_UNLOCK(irq_state) \
    if(threading) \
        simple_unlock(&heap_lock); \
    else \
        spin_unlock_irqrstor(&heap_lock, irq_state)


/* called from vm_init. The heap should already be mapped in at this point,
 * we just do a little housekeeping to set up the data structure.
*/
status_t heap_init(addr_t new_heap_base, size_t new_heap_size)
{
    const uint32 page_entries = PAGE_SIZE / sizeof(struct heap_page);
    /* set some global pointers */
    heap_alloc_table = (struct heap_page *)new_heap_base;
    heap_size = ((uint64)new_heap_size * page_entries / (page_entries + 1)) & ~(PAGE_SIZE-1);
    /** heap_size = new_heap_size - PAGE_SIZE;
    *** use this for heaps less or equal 8Mb in size.
    ***/
    heap_base = (addr_t)heap_alloc_table + PAGE_ALIGN(heap_size / page_entries);
    heap_base_ptr = heap_base;
#ifdef MAKE_NOISE
    kprint("heap_alloc_table = %p, heap_base = 0x%lx, heap_size = 0x%lx\n", heap_alloc_table, heap_base, heap_size);
#endif

    /* zero out the heap alloc table at the base of the heap */
    memset((void *)heap_alloc_table, 0, (heap_size / PAGE_SIZE) * sizeof(struct heap_page));

    /* init simple lock */
    simple_lock_init(&heap_lock);

    return NO_ERROR;
}

status_t heap_init_postthread(kernel_args_t *ka)
{
    threading = true; /* threading is functional */

    return NO_ERROR;
}

status_t heap_init_postsem(kernel_args_t *ka)
{
    /* nothing to do for now */

    return NO_ERROR;
}

static char *raw_alloc(uint32 size, int bin_index)
{
    addr_t new_heap_ptr;
    char *retval;
    struct heap_page *page;
    addr_t addr;

    /* compute new heap size */
    new_heap_ptr = heap_base_ptr + PAGE_ALIGN(size);
    if(new_heap_ptr > heap_base + heap_size) {
        panic("heap overgrew itself!\n");
    }

    /* walk from current heap_base_ptr to new heap_base_ptr and
     * mark space as allocated
     */
    for(addr = heap_base_ptr; addr < new_heap_ptr; addr += PAGE_SIZE) {
        page = &heap_alloc_table[(addr - heap_base) / PAGE_SIZE];
        page->in_use = 1;
        page->cleaning = 0;
        page->bin_index = bin_index;
        if (bin_index < bin_count && bins[bin_index].element_size < PAGE_SIZE)
            page->free_count = PAGE_SIZE / bins[bin_index].element_size;
        else
            page->free_count = 1;
    }

    retval = (char *)heap_base_ptr;
    heap_base_ptr = new_heap_ptr;
    return retval;
}

void *kmalloc(size_t size)
{
    void *address = NULL;
    int bin_index;
    struct heap_page *page;
    unsigned long irq_state = 0;
    uint32 i;

#if MAKE_NOIZE
    kprint("kmalloc: asked to allocate size %d\n", size);
#endif

    /* lock */
    HEAP_LOCK(irq_state);

    /* find a bin of suitable size */
    for (bin_index = 0; bin_index < bin_count; bin_index++)
        if (size <= bins[bin_index].element_size)
            break;

    /* if requested memory block is too big,
     * no bin of suitable size discovered
     */
    if (bin_index == bin_count) {
        /* XXX fix the raw alloc later. */
        panic("kmalloc: asked to allocate too much for now!\n");
        goto out;
    } else {
        /* allocate space */
        if (bins[bin_index].free_list != NULL) {
            address = bins[bin_index].free_list;
            bins[bin_index].free_list = (void *)(*(addr_t *)bins[bin_index].free_list);
            bins[bin_index].free_count--;
        } else {
            if (bins[bin_index].raw_count == 0) {
                bins[bin_index].raw_list = raw_alloc(bins[bin_index].grow_size, bin_index);
                bins[bin_index].raw_count = bins[bin_index].grow_size / bins[bin_index].element_size;
            }

            bins[bin_index].raw_count--;
            address = bins[bin_index].raw_list;
            bins[bin_index].raw_list += bins[bin_index].element_size;
        }

        bins[bin_index].alloc_count++;
        page = &heap_alloc_table[((addr_t)address - heap_base) / PAGE_SIZE];
        page[0].free_count--;
#if MAKE_NOIZE
        kprint("kmalloc0: page 0x%x: bin_index %d, free_count %d\n", page, page->bin_index, page->free_count);
#endif
        for(i = 1; i < bins[bin_index].element_size / PAGE_SIZE; i++) {
            page[i].free_count--;
#if MAKE_NOIZE
            kprint("kmalloc1: page 0x%x: bin_index %d, free_count %d\n", page[i], page[i].bin_index, page[i].free_count);
#endif
        }
    }

out:
    /* unlock */
    HEAP_UNLOCK(irq_state);

#if MAKE_NOIZE
    kprint("kmalloc: asked to allocate size %d, returning ptr = %p\n", size, address);
#endif
    return address;
}

void kfree(void *address)
{
    struct heap_page *page;
    struct heap_bin *bin;
    unsigned long irq_state = 0;
    uint32 i;

    /* ignore NULL pointers */
    if (address == NULL)
         return;

    /* if given address lies out of heap */
    if ((addr_t)address < heap_base || (addr_t)address >= (heap_base + heap_size))
        panic("kfree: asked to free invalid address %p\n", address);

    /* lock */
    HEAP_LOCK(irq_state);

#if MAKE_NOIZE
    kprint("kfree: asked to free at ptr = %p\n", address);
#endif

    /* get page */
    page = &heap_alloc_table[((addr_t)address - heap_base) / PAGE_SIZE];

#if MAKE_NOIZE
    kprint("kfree: page 0x%x: bin_index %d, free_count %d\n", page, page->bin_index, page->free_count);
#endif

    if(page[0].bin_index >= bin_count)
        panic("kfree: page %p: invalid bin_index %d\n", page, page->bin_index);

    /* get bin */
    bin = &bins[page[0].bin_index];

    if(bin->element_size <= PAGE_SIZE && (addr_t)address % bin->element_size != 0)
        panic("kfree: passed invalid pointer %p! Supposed to be in bin for esize 0x%x\n", address, bin->element_size);

    for(i = 0; i < bin->element_size / PAGE_SIZE; i++) {
        if(page[i].bin_index != page[0].bin_index)
            panic("kfree: not all pages in allocation match bin_index\n");
        page[i].free_count++;
    }

#if PARANOID_KFREE
    /* walk the free list on this bin to make sure this address doesn't exist already */
    {
        addr_t *temp;
        for(temp = bin->free_list; temp != NULL; temp = (addr_t *)*temp) {
            if(temp == (addr_t *)address) {
                panic("kfree: address %p already exists in bin free list\n", address);
            }
        }
    }
#endif

#if WIPE_KFREE
    memset(address, 0x99, bin->element_size);
#endif

    *(addr_t *)address = (addr_t)bin->free_list;
    bin->free_list = address;
    bin->alloc_count--;
    bin->free_count++;

    /* unlock */
    HEAP_UNLOCK(irq_state);
}

void kfree_and_null(void **address)
{
    if(!address || !*address) return;
    kfree(*address);
    *address = NULL;
}

void *kmalloc_pages(size_t npages)
{
    void *p;

    p = kmalloc(npages * PAGE_SIZE);
    if (!p)
        return NULL;

    /* check page alignment in debug kernels */
    ASSERT_MSG(!((addr_t)p % PAGE_SIZE), "allocated memory is not page aligned!\n");

    return p;
}
