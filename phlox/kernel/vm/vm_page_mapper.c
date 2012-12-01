/*
* Copyright 2007-2012, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <sys/debug.h>
#include <phlox/errors.h>
#include <phlox/avl_tree.h>
#include <phlox/list.h>
#include <phlox/sem.h>
#include <phlox/mutex.h>
#include <phlox/vm_private.h>
#include <phlox/vm.h>
#include <phlox/vm_page_mapper.h>

/* A little notification for future */
#warning "TODO comments here"

/* mapping descriptor */
typedef struct mapping_desc {
    list_elem_t lst_elem;    /* list element */
    avl_tree_node_t tree_n;  /* AVL tree node */
    uint ref_count;          /* references count */
    addr_t vaddr;            /* virtual address */
    addr_t paddr;            /* physical address */
} mapping_desc_t;

/* Mappings pool */
static addr_t map_pool_base;       /* Mappings pool base address */
static size_t map_pool_size;       /* Mappings pool size         */
static size_t map_pool_chunksize;  /* Mappings pool chunk size   */
static map_chunk_func_t map_chunk; /* Map chunk function         */

/*
 * Array of descriptors for chunks in mappings pool.
 * Each descriptor corresponds to one chunk in mappings pool.
 */
static mapping_desc_t *mappings;
static size_t mappings_count;

/* Bookkeeping structures */
static xlist_t used_mappings;  /* List of used mapping descriptors */
static xlist_t free_mappings;  /* List of available mapping descriptors */
static avl_tree_t mappings_tree; /* Tree of mappings. Each used mapping
                                  * descriptor added to this tree.
                                  */

static mutex_t map_pool_mutex; /* Mappings pool mutex */
/* TODO: add semapore for waiters */

/* mappings compare routine for AVL tree */
static int compare_mapping_desc(const void *d1, const void *d2)
{
    if( ((mapping_desc_t *)d1)->paddr > ((mapping_desc_t *)d2)->paddr )
        return 1;
    if( ((mapping_desc_t *)d1)->paddr < ((mapping_desc_t *)d2)->paddr )
        return -1;
    return 0;
}

/**** Locally used routines ****/

/* Find mapping in AVL tree by physical address */
static mapping_desc_t *tree_find_mapping(addr_t paddr, avl_tree_index_t *where)
{
    mapping_desc_t *look4;

    look4 = containerof(&paddr, mapping_desc_t, paddr);

    return avl_tree_find(&mappings_tree, look4, where);
}

/* insert new mapping into tree */
static inline void tree_insert_mapping(mapping_desc_t *mapping, avl_tree_index_t where)
{
    avl_tree_insert(&mappings_tree, mapping, where);
}

/* remove mapping from tree */
static inline int tree_remove_mapping(mapping_desc_t *mapping)
{
    return (int)avl_tree_remove(&mappings_tree, mapping);
}

/* add mapping to list */
static inline void list_add_mapping(xlist_t *list, mapping_desc_t *mapping)
{
    xlist_add(list, &mapping->lst_elem);
}

/* remove mapping from list */
static inline int list_remove_mapping(xlist_t *list, mapping_desc_t *mapping)
{
    return (int)xlist_remove_unsafe(list, &mapping->lst_elem);
}

/* extract first mapping from list */
static inline mapping_desc_t *list_extract_first_mapping(xlist_t *list)
{
    list_elem_t *tmp = xlist_extract_first(list);
    return containerof(tmp, mapping_desc_t, lst_elem);
}


/* page mapper init */
status_t vm_page_mapper_init(kernel_args_t *kargs, addr_t *pool_base, size_t pool_size,
                             size_t pool_chunksize, size_t pool_align,
                             map_chunk_func_t map_chunk_func)
{
    uint i;

    /* store local variables */
    map_pool_size = pool_size;
    map_pool_chunksize = pool_chunksize;
    mappings_count = map_pool_size / map_pool_chunksize;
    map_chunk = map_chunk_func;

    /* Mappings pool size must be chunksize aligned! */
    if (map_pool_size % map_pool_chunksize) {
        panic("vm_page_mapper_init: map_pool_size is not map_pool_chunksize aligned.");
        return ERR_INVALID_ARGS;
    }

    /*
     * Reserve virtual space for the mappings pool
     * We reserve (pool_align - PAGE_SIZE) bytes more, so that we
     * can guarantee to align the base address to pool_align.
     */
    map_pool_base = vm_alloc_vspace_from_kargs(kargs, map_pool_size + pool_align - PAGE_SIZE);
    if (!map_pool_base)
        return ERR_NO_MEMORY;

    /* align the base address to pool_align */
    map_pool_base = (map_pool_base + pool_align - 1) / pool_align * pool_align;
    *pool_base = map_pool_base;

    /* allocate memory for mapping descriptors */
    mappings = (mapping_desc_t *)vm_alloc_from_kargs( kargs,
                       mappings_count * sizeof(mapping_desc_t),
                       VM_PROT_KERNEL_DEFAULT);

    if (!mappings)
        return ERR_NO_MEMORY;

    /* clear allocated memory */
    memset(mappings, 0, mappings_count * sizeof(mapping_desc_t));

    /* init bookkeeping structures */
    avl_tree_create( &mappings_tree,
                     compare_mapping_desc,
                     sizeof(mapping_desc_t),
                     offsetof(mapping_desc_t, tree_n) );
    xlist_init(&used_mappings);
    xlist_init(&free_mappings);

    /* add chunks to free list */
    for(i=0; i < mappings_count; i++) {
        mappings[i].vaddr = map_pool_base + (i * map_pool_chunksize);
        list_add_mapping(&free_mappings, &mappings[i]);
    }

    /* mutex init, it is not actual mutex creation */
    mutex_init(&map_pool_mutex);

    return NO_ERROR;
}

/* final stage of page mapper init */
status_t vm_page_mapper_init_final(kernel_args_t *kargs)
{
    aspace_id kid = vm_get_kernel_aspace_id();
    object_id id;
    status_t err;

    /* create memory hole for mappings area */
    err = vm_create_memory_hole(kid, map_pool_base, map_pool_size);
    if(err != NO_ERROR)
        return err;

    /* create mapping descriptors object ... */
    id = vm_create_virtmem_object("kernel_mapping_descr", kid, (addr_t)mappings,
                                  mappings_count * sizeof(mapping_desc_t),
                                  VM_OBJECT_PROTECT_ALL);
    if(id == VM_INVALID_OBJECTID)
        return ERR_GENERAL;

    /* ... and its mapping */
    err = vm_map_object_exactly(kid, id, VM_PROT_KERNEL_ALL, (addr_t)mappings);
    if(err != NO_ERROR)
        return err;

    /* init page reference counters */
    err = vm_page_init_wire_counters((addr_t)mappings, mappings_count * sizeof(mapping_desc_t));

    return NO_ERROR;
}

/* post-semaphore init stage */
status_t vm_page_mapper_init_post_sema(kernel_args_t *kargs)
{
    return mutex_create(&map_pool_mutex, "map_pool_mutex");
}

/* get physical page */
status_t vm_pmap_get_ppage(addr_t pa, addr_t *va, bool can_wait)
{
    addr_t paddr = ROUNDOWN(pa, map_pool_chunksize); /* chunk physical address */
    avl_tree_index_t where;
    mapping_desc_t *md;


restart:

    /* grab mutex */
    mutex_lock(&map_pool_mutex);

    /* see if the chunk is already mapped */
    md = tree_find_mapping(paddr, &where);
    if (md) {
        /* ok. return it to caller */
        md->ref_count++;
        *va = md->vaddr + pa % map_pool_chunksize;
        mutex_unlock(&map_pool_mutex);
        return NO_ERROR;
    }

    /* get free mapping descriptor */
    md = list_extract_first_mapping(&free_mappings);
    if(md == NULL) {
        /* there is no free mapping slots */
        if(!can_wait) {
            /* punt back to the caller and let them handle this */
            mutex_unlock(&map_pool_mutex);
            return ERR_NO_MEMORY;
        } else {
            mutex_unlock(&map_pool_mutex);
            /*
             * TODO: Acquire semaphore and wait
            */
            goto restart;
        }
    }

    /* prepare descriptor before mapping chunk */
    md->ref_count++;
    md->paddr = paddr;
    *va = md->vaddr + pa % map_pool_chunksize;
    /* add it to used mappings list */
    list_add_mapping(&used_mappings, md);
    /* ... and into mappings tree */
    tree_insert_mapping(md, where);

    /* map chunk */
    if ( map_chunk(md->paddr, md->vaddr) ) {
        ASSERT_MSG(0, "vm_pmap_get_ppage: map_chunk failed");
        /* release mutex */
        mutex_unlock(&map_pool_mutex);
        return ERR_VM_GENERAL;
    }

    /* release mutex */
    mutex_unlock(&map_pool_mutex);

    return NO_ERROR;
}

/* put physical page */
status_t vm_pmap_put_ppage(addr_t va)
{
    mapping_desc_t *md;

    /* check inputs */
    if(va < map_pool_base || va >= map_pool_base + map_pool_size)
        panic("someone called vm_pmap_put_ppage on an invalid va 0x%lx\n", va);

    va -= map_pool_base; /* chunk address in mappings pool */

    /* grab mutex */
    mutex_lock(&map_pool_mutex);

    /* get descriptor */
    md = &mappings[va / map_pool_chunksize];
    if(md->ref_count == 0) {
        /** reference count cannot be 0 here ! **/
        mutex_unlock(&map_pool_mutex);
        panic("vm_pmap_put_ppage called on page at va 0x%lx which is not checked out\n", va);
        return ERR_VM_GENERAL;
    }

    /* do bookkeeping */
    if(--md->ref_count == 0) {
        /* that was last reference */

        /* remove mapping descriptor from used mappings list */
        list_remove_mapping(&used_mappings, md);
        /* ... and from tree */
        if (!tree_remove_mapping(md))
            panic("vm_pmap_put_ppage: mapping is not in AVL tree!\n");

        /* put it to free mappings list */
        list_add_mapping(&free_mappings, md);

        /* TODO: if there is waiters for free descriptors do wake up! */
    }

    /* release mutex */
    mutex_unlock(&map_pool_mutex);

    return NO_ERROR;
}
