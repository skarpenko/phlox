/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
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

/* Spinlock for operations on address spaces list */
static spinlock_t aspaces_lock;

/* Kernel address space */
static vm_address_space_t *kernel_aspace = NULL;


/*** Locally used routines ***/

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
    xlist_remove(&aspaces_list, &aspace->list_node);

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
    /* TODO: avl_tree_create(aspace->mmap.mappings_tree) */
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
status_t vm_address_spaces_init(kernel_args_t *kargs); /* keep silence */
status_t vm_address_spaces_init(kernel_args_t *kargs)
{
    /* set initial value */
    next_aspace_id = 1;

    /* init spinlock for list access */
    spin_init(&aspaces_lock);

    /* init address spaces list */
    xlist_init(&aspaces_list);

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
