/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <sys/debug.h>
#include <phlox/param.h>
#include <phlox/errors.h>
#include <phlox/heap.h>
#include <phlox/list.h>
#include <phlox/avl_tree.h>
#include <phlox/atomic.h>
#include <phlox/spinlock.h>
#include <phlox/thread.h>
#include <phlox/arch/vm_translation_map.h>
#include <phlox/vm_private.h>
#include <phlox/vm_page.h>
#include <phlox/vm.h>


/* Redefinition for convenience */
typedef xlist_t aspace_list_t;

/* Next available address space id */
static vuint next_aspace_id;

/* List of address spaces */
static aspace_list_t aspaces_list;

/* AVL tree of address spaces for fast find by id */
static avl_tree_t aspaces_tree;

/* Spinlock for operations on address spaces list and tree */
static spinlock_t aspaces_lock;

/* Kernel address space */
static vm_address_space_t *kernel_aspace = NULL;


/*** Locally used routines ***/

/* compare routine for address spaces AVL tree */
static int compare_aspace_id(const void *a1, const void *a2)
{
    if( ((vm_address_space_t *)a1)->id > ((vm_address_space_t *)a2)->id )
        return 1;
    if( ((vm_address_space_t *)a1)->id < ((vm_address_space_t *)a2)->id )
        return -1;
    return 0;
}

/* compare routine for mappings AVL tree */
static int compare_mapping(const void *m1, const void *m2)
{
    if( ((vm_mapping_t *)m1)->start > ((vm_mapping_t *)m2)->end )
        return 1;
    if( ((vm_mapping_t *)m1)->end < ((vm_mapping_t *)m2)->start )
        return -1;

    /* Control goes here only if two mappings are equal or
     * overlaps. That is possible only during mapping search
     * by address.
     * Overlapped mappings are forbidden!
     */
    return 0;
}

/* returns available address space id */
static aspace_id get_next_aspace_id(void)
{
    aspace_id retval;

    /* atomically increment and get previous value */
    retval = (aspace_id)atomic_inc_ret(&next_aspace_id);
    if(retval == VM_INVALID_ASPACEID)
        panic("No available address space IDs!");
    /* TODO: Implement better approach for reliability? */

    return retval;
}

/* put address space to end of the list */
static void put_aspace_to_list(vm_address_space_t *aspace)
{
    unsigned long irqs_state;

    /* acquire lock before touching list */
    irqs_state = spin_lock_irqsave(&aspaces_lock);

    /* add item */
    xlist_add_last(&aspaces_list, &aspace->list_node);

    /* additionally put it it into tree */
    if(!avl_tree_add(&aspaces_tree, aspace))
      panic("put_aspace_to_list(): failed to add aspace into tree!\n");

    /* release lock */
    spin_unlock_irqrstor(&aspaces_lock, irqs_state);
}

/* removes address space from list */
static void remove_aspace_from_list(vm_address_space_t *aspace)
{
    unsigned long irqs_state;

    /* acquire lock before */
    irqs_state = spin_lock_irqsave(&aspaces_lock);

    /* remove item */
    xlist_remove_unsafe(&aspaces_list, &aspace->list_node);

    /* and remove from tree */
    if(!avl_tree_remove(&aspaces_tree, aspace))
      panic("remove_aspace_from_list(): failed to remove aspace from tree!\n");

    /* release lock */
    spin_unlock_irqrstor(&aspaces_lock, irqs_state);
}

/* common routine for creating kernel or user address space */
static vm_address_space_t *create_aspace_common(const char* name, addr_t base, size_t size, bool kernel)
{
    vm_address_space_t *aspace;

    /* address space must have size */
    if(size == 0)
        return NULL;

    /* allocate address space structure in heap */
    aspace = (vm_address_space_t *)kmalloc(sizeof(vm_address_space_t));
    if(!aspace)
        return NULL;

    /* if address space has name - copy it to structure field */
    if(name) {
        aspace->name = kstrdup(name);
        if(!aspace->name)
            goto error;
    } else {
        aspace->name = NULL;
    }

    /* create proper translation map */
    if(kernel) {
       if(vm_tmap_kernel_create(&aspace->tmap) != NO_ERROR)
           goto error;
    } else {
       if(vm_tmap_create(&aspace->tmap) != NO_ERROR)
           goto error;
    }

    /* init memory map fields */
    aspace->mmap.base = base;
    aspace->mmap.size = size;
    xlist_init(&aspace->mmap.mappings_list);
    avl_tree_create( &aspace->mmap.mappings_tree, compare_mapping,
                     sizeof(vm_mapping_t),
                     offsetof(vm_mapping_t, tree_node) );
    aspace->mmap.aspace = aspace;

    /* init address space fields */
    spin_init(&aspace->lock);
    aspace->state = VM_ASPACE_STATE_NORMAL;
    aspace->ref_count = 0;
    aspace->faults_count = 0;
    aspace->id = get_next_aspace_id();

    /* return to caller */
    return aspace;

error:
    /* return memory to heap on error */
    if(aspace->name)
        kfree(aspace->name);
    kfree(aspace);

    return NULL; /* failed */
}

/* locates free gap in memory map of address space (no lock acquired for access) */
static bool locate_memory_gap(vm_memory_map_t *mmap, size_t size, addr_t *base_vaddr)
{
    list_elem_t *item;
    vm_mapping_t *mapping;
    addr_t start, end;

    /* fetch first item of mappings list */
    item = xlist_peek_first(&mmap->mappings_list);

    /* if no mappings, check that requested size
     * fits into address space
     */
    if(item == NULL) {
        if(size <= mmap->size) {
            *base_vaddr = mmap->base;
            return true;
        } else {
            return false;
        }
    }

    /* search for gap of sufficient size */
    start = mmap->base;
    while(item != NULL) {
        mapping = containerof(item, vm_mapping_t, list_node);
        end = mapping->start - 1;
        if(size <= end - start + 1) {
            /* gap found! */
            *base_vaddr = start;
            return true;
        }

        /* step to next mapping */
        start = mapping->end + 1;
        item = xlist_peek_next(item);
    }

    /* no gap of sufficient size found between mappings.
     * so.... check space after last mapping till the end
     * of memory map.
     */
    end = mmap->base + mmap->size - 1;
    if(size <= end - start + 1) {
        /* gap size is enought */
        *base_vaddr = start;
        return true;
    }

    /* no gap found */
    return false;
}

/* puts mapping into mappings tree and list, keeping list sorted
 * (no lock acquired for address space access)
 */
static bool put_mapping_to_aspace(vm_address_space_t *aspace, vm_mapping_t *mapping)
{
    avl_tree_index_t where;
    vm_mapping_t *parent;

    /* get "where" index and ensure that add of mapping is permitted */
    if(avl_tree_find(&aspace->mmap.mappings_tree, mapping, &where) != NULL)
        return false;

    /* put mapping into tree */
    avl_tree_insert(&aspace->mmap.mappings_tree, mapping, where);

    /* if that is the only mapping in address space,
     * just add to list and exit.
     */
    if(!where.node) {
        xlist_add_first(&aspace->mmap.mappings_list, &mapping->list_node);
        return true;
    }

    /* fetch parent tree node from "where" index */
    parent = containerof(where.node, vm_mapping_t, tree_node);

    /* add to a proper position in list */
    if(!where.child) {
        xlist_insert_before_unsafe(&aspace->mmap.mappings_list, &parent->list_node,
                                   &mapping->list_node);
    } else {
        xlist_insert_after_unsafe(&aspace->mmap.mappings_list, &parent->list_node,
                                  &mapping->list_node);
    }

    return true;
}

/* removes mapping from address space (no lock acquired before) */
static bool remove_mapping_from_aspace(vm_address_space_t *aspace, vm_mapping_t *mapping)
{
    /* remove from the tree */
    if(avl_tree_remove(&aspace->mmap.mappings_tree, mapping) == NULL)
        return false;

    /* ... and from the list */
    if(!xlist_remove(&aspace->mmap.mappings_list, &mapping->list_node))
        return false;

    return true;
}


/*** Public routines ***/

/* Module initialization routine */
status_t vm_address_spaces_init(kernel_args_t *kargs)
{
    /* set initial value */
    next_aspace_id = 1;

    /* init spinlock for list access */
    spin_init(&aspaces_lock);

    /* init address spaces list */
    xlist_init(&aspaces_list);

    /* init address spaces AVL tree */
    avl_tree_create( &aspaces_tree, compare_aspace_id,
                     sizeof(vm_address_space_t),
                     offsetof(vm_address_space_t, tree_node) );

    return NO_ERROR;
}

/* creates mapping structure and puts it into memory map of address space */
status_t vm_aspace_create_mapping(vm_address_space_t *aspace, size_t size, uint npg_align, vm_mapping_t **mapping)
{
    addr_t base_addr;
    size_t size_pad;

    /* check arguments */
    if(size == 0)
        return ERR_INVALID_ARGS;

    /* align chunk size and size padding */
    if(npg_align < 2) {
        npg_align = 0;
        size_pad  = 0;
    } else {
        npg_align *= PAGE_SIZE;
        size_pad = npg_align - PAGE_SIZE;
    }

    /* locate gap of requested size */
    if(!locate_memory_gap(&aspace->mmap, size + size_pad, &base_addr))
        return ERR_VM_NO_MAPPING_GAP;

    /* align base address if needed */
    if(npg_align)
        base_addr = (base_addr + npg_align - 1) / npg_align * npg_align;

    /* create mapping exactly */
    return vm_aspace_create_mapping_exactly(aspace, base_addr, size, mapping);
}

/* creates mapping structure with fixed base address and puts it into memory map of address space */
status_t vm_aspace_create_mapping_exactly(vm_address_space_t *aspace, addr_t base, size_t size, vm_mapping_t **mapping)
{
    /* check arguments */
    if(base < aspace->mmap.base || size > aspace->mmap.size || size == 0)
        return ERR_INVALID_ARGS;

    /* allocate memory for structure */
    *mapping = (vm_mapping_t *)kmalloc(sizeof(vm_mapping_t));
    ASSERT_MSG(*mapping != NULL, "vm_aspace_create_mapping_exactly(): no memory!");
    if(*mapping == NULL)
        return ERR_NO_MEMORY;

    /* init structure fields */
    (*mapping)->start   = base;
    (*mapping)->end     = base + size - 1;
    (*mapping)->mmap    = &aspace->mmap;
    (*mapping)->object  = NULL;
    (*mapping)->offset  = 0;
    (*mapping)->type    = VM_MAPPING_TYPE_HOLE;
    (*mapping)->protect = 0;
    xlist_elem_init(&(*mapping)->list_node);
    xlist_elem_init(&(*mapping)->obj_list_node);

    /* put it into memory map of address space */
    if(!put_mapping_to_aspace(aspace, *mapping)) {
        kfree(*mapping);
        return ERR_VM_BAD_ADDRESS;
    }

    /* success */
    return NO_ERROR;
}

/* deletes mapping from address space */
void vm_aspace_delete_mapping(vm_address_space_t *aspace, vm_mapping_t *mapping)
{
    if(!remove_mapping_from_aspace(aspace, mapping))
        panic("vm_aspace_delete_mapping(): failed to remove mapping!");
    kfree(mapping);
}

/* get mapping by virtual address within address space */
status_t vm_aspace_get_mapping(vm_address_space_t *aspace, addr_t vaddr, vm_mapping_t **mapping)
{
    vm_mapping_t dummy;

    /* check specified address */
    if(vaddr < aspace->mmap.base || vaddr > aspace->mmap.base + aspace->mmap.size - 1)
        return ERR_VM_BAD_ADDRESS;

    /* fill dummy fields */
    dummy.start = vaddr;
    dummy.end   = vaddr;

    /* search tree */
    *mapping = avl_tree_find(&aspace->mmap.mappings_tree, &dummy, NULL);
    if(*mapping == NULL)
        return ERR_VM_NO_MAPPING;

    /* success */
    return NO_ERROR;
}

/* create kernel address space (called only once) */
aspace_id vm_create_kernel_aspace(const char* name, addr_t base, size_t size)
{
    vm_address_space_t *aspace;
    aspace_id id;

    ASSERT_MSG(name != NULL && strlen(name) <= SYS_MAX_OS_NAME_LEN,
        "vm_create_kernel_aspace: kernel address space name is invalid!\n");

    /* ensure that kernel space is not created before */
    if(kernel_aspace)
        panic("vm_create_kernel_aspace: kernel space already exists!\n");

    /* create kernel address space */
    aspace = create_aspace_common(name, base, size, true);
    if(!aspace)
        return VM_INVALID_ASPACEID;

    id = aspace->id;
    kernel_aspace = aspace; /* store into global variable */

    /* add to address spaces list */
    put_aspace_to_list(aspace);

    return id; /* return id to caller */
}

/* create ordinary address space */
aspace_id vm_create_aspace(const char* name, addr_t base, size_t size)
{
    vm_address_space_t *aspace;
    aspace_id id;

    /* check name length */
    if(name != NULL && strlen(name) > SYS_MAX_OS_NAME_LEN)
        return VM_INVALID_ASPACEID;

    /* check that name is unique */
    if(vm_find_aspace_by_name(name) != VM_INVALID_ASPACEID)
        return VM_INVALID_ASPACEID;

    /* create address space */
    aspace = create_aspace_common(name, base, size, false);
    if(!aspace)
        return VM_INVALID_ASPACEID;

    id = aspace->id;

    /* add to address spaces list */
    put_aspace_to_list(aspace);

    return id; /* return id */
}

/* TODO: status_t vm_delete_aspace(aspace_id aid) */

/* returns kernel address space */
vm_address_space_t *vm_get_kernel_aspace(void)
{
    ASSERT_MSG(kernel_aspace, "Kernel address space is not created!");

    /* increase references count */
    atomic_inc(&kernel_aspace->ref_count);
    /* ... and return to caller */
    return kernel_aspace;
}

/* returns kernel address space id */
aspace_id vm_get_kernel_aspace_id(void)
{
    ASSERT_MSG(kernel_aspace, "Kernel address space is not created!");

    return kernel_aspace->id;
}

/* returns current user address space */
vm_address_space_t *vm_get_current_user_aspace(void)
{
    thread_t *th;

    /* kernel threads operates in kernel address space and
     * owned by kernel process there is no user address space
     * exists in this case.
     */
    if(thread_is_kernel_thread())
        return NULL;

    /* take current thread */
    th = thread_get_current_thread();

    /* locate and return address space to caller */
    return vm_get_aspace_by_id(th->process->aid);
}

/* returns current user address space id */
aspace_id vm_get_current_user_aspace_id(void)
{
    thread_t *th;

    /* no user address space for kernel threads */
    if(thread_is_kernel_thread())
        return VM_INVALID_ASPACEID;

    /* take current thread... */
    th = thread_get_current_thread();

    /* ... and return address space id stored inside process structure */
    return th->process->aid;
}

/* returns address space by its id */
vm_address_space_t* vm_get_aspace_by_id(aspace_id aid)
{
    vm_address_space_t temp_aspace, *aspace;
    unsigned long irqs_state;

    temp_aspace.id = aid;

    /* acquire lock before accessing tree */
    irqs_state = spin_lock_irqsave(&aspaces_lock);

    /* search tree */
    aspace = avl_tree_find(&aspaces_tree, &temp_aspace, NULL);

    /* increase references count */
    if(aspace)
        atomic_inc(&aspace->ref_count);

    /* release lock */
    spin_unlock_irqrstor(&aspaces_lock, irqs_state);

    return aspace;
}

/* put previously taken address space */
void vm_put_aspace(vm_address_space_t *aspace)
{
    /* decrease references count */
    atomic_dec(&aspace->ref_count);
    /* TODO: implement additional functionality */
}

/* returns address space id by its name */
aspace_id vm_find_aspace_by_name(const char *name)
{
    unsigned long irqs_state;
    list_elem_t *item;
    vm_address_space_t *aspace;
    aspace_id id = VM_INVALID_ASPACEID;

    /* return invalid id if NULL passed */
    if(!name)
        return VM_INVALID_ASPACEID;

    /* acquire lock before touching list */
    irqs_state = spin_lock_irqsave(&aspaces_lock);

    /* start search from first list item */
    item = xlist_peek_first(&aspaces_list);
    while(item) {
        /* convert node to address space structure */
        aspace = containerof(item, vm_address_space_t, list_node);

        /* we skip unnamed address spaces and spaces with
         * deletion state.
         */
        if(aspace->state != VM_ASPACE_STATE_DELETION &&
           aspace->name && !strcmp(aspace->name, name))
        {
            id = aspace->id;
            break; /* search successful */
        }

        /* step to next item */
        item = xlist_peek_next(item);
    }

    /* release lock */
    spin_unlock_irqrstor(&aspaces_lock, irqs_state);

    return id;
}

/* creates memory hole */
status_t vm_create_memory_hole(aspace_id aid, addr_t base, size_t size)
{
    vm_address_space_t *aspace;
    vm_mapping_t *mapping;
    status_t err;

    /* try to get address space */
    aspace = vm_get_aspace_by_id(aid);
    if(aspace == NULL)
        return ERR_VM_INVALID_ASPACE;

    /* acquire lock */
    spin_lock(&aspace->lock);
#warning "vm_create_memory_hole(): replace spinlocks!"

    /* create mapping, by default it is memory hole */
    err = vm_aspace_create_mapping_exactly(aspace, base, size, &mapping);

    /* release lock */
    spin_unlock(&aspace->lock);

    /* put address space back */
    vm_put_aspace(aspace);

    return err;
}

/* deletes memory hole */
status_t vm_delete_memory_hole(aspace_id aid, addr_t vaddr)
{
    vm_address_space_t *aspace;
    vm_mapping_t *mapping;
    status_t err;

    /* get specified address space */
    aspace = vm_get_aspace_by_id(aid);
    if(aspace == NULL)
        return ERR_VM_INVALID_ASPACE;

    /* acquire lock */
    spin_lock(&aspace->lock);
#warning "vm_delete_memory_hole(): replace spinlocks!"

    /* get mapping at specified address. it must be memory hole. */
    err = vm_aspace_get_mapping(aspace, vaddr, &mapping);
    if(err == NO_ERROR) {
        if(mapping->type != VM_MAPPING_TYPE_HOLE) {
           err = ERR_VM_BAD_ADDRESS;
        } else {
            vm_aspace_delete_mapping(aspace, mapping);
        }
    }

    /* release lock */
    spin_unlock(&aspace->lock);

    /* put address space back */
    vm_put_aspace(aspace);

    return err;
}
