/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <sys/debug.h>
#include <phlox/errors.h>
#include <phlox/heap.h>
#include <phlox/list.h>
#include <phlox/avl_tree.h>
#include <phlox/atomic.h>
#include <phlox/spinlock.h>
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

/* create kernel address space (called only once) */
aspace_id vm_create_kernel_aspace(const char* name, addr_t base, size_t size)
{
    vm_address_space_t *aspace;
    aspace_id id;

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

/* TODO: vm_address_space_t *vm_get_current_user_aspace(void) */
/* TODO: aspace_id vm_get_current_user_aspace_id(void) */

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

    /* release lock */
    spin_unlock_irqrstor(&aspaces_lock, irqs_state);

    /* increase references count */
    if(aspace)
        atomic_inc(&aspace->ref_count);

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
