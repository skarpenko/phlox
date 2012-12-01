/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <phlox/errors.h>
#include <phlox/heap.h>
#include <phlox/list.h>
#include <phlox/atomic.h>
#include <phlox/spinlock.h>
#include <phlox/vm_page.h>
#include <phlox/vm.h>


/* Redefinition for convenience */
typedef xlist_t object_list_t;

/* Next available object id */
static vuint next_object_id;

/* Objects list */
static object_list_t objects_list;

/* Spinlock for operations on objects list */
static spinlock_t objects_lock;


/*** Locally used routines ***/

/* returns available object id */
static object_id get_next_object_id(void)
{
    object_id retval;

    /* atomically increment and get previous value */
    retval = (object_id)atomic_inc_ret(&next_object_id);
    if(retval == VM_INVALID_OBJECTID)
        panic("No available VM object IDs!");
    /* TODO: Implement better approach for reliability? */

    return retval;
}

/* put object to end of the list */
static void put_object_to_list(vm_object_t *object)
{
    unsigned long irqs_state;

    /* acquire lock before touching list */
    irqs_state = spin_lock_irqsave(&objects_lock);

    /* add item */
    xlist_add_last(&objects_list, &object->list_node);

    /* release lock */
    spin_unlock_irqrstor(&objects_lock, irqs_state);
}

/* remove object from list */
static void remove_object_from_list(vm_object_t *object)
{
    unsigned long irqs_state;

    /* acquire lock befor modifying list */
    irqs_state = spin_lock_irqsave(&objects_lock);

    /* remove item */
    xlist_remove(&objects_list, &object->list_node);

    /* release lock */
    spin_unlock_irqrstor(&objects_lock, irqs_state);
}


/*** Public routines ***/

/* Module initialization routine */
status_t vm_objects_init(kernel_args_t *kargs); /* keep silence */
status_t vm_objects_init(kernel_args_t *kargs)
{
   /* set initially available object id */
    next_object_id = 1;

    /* init spinlock for list access */
    spin_init(&objects_lock);

    /* init objects list */
    xlist_init(&objects_list);

    return NO_ERROR;
}
