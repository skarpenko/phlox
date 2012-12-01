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
#include <phlox/vm_page.h>
#include <phlox/vm.h>


/* Redefinition for convenience */
typedef xlist_t object_list_t;

/* Next available object id */
static vuint next_object_id;

/* Objects list */
static object_list_t objects_list;

/* AVL tree of objects for fast find by id */
static avl_tree_t objects_tree;

/* Spinlock for operations on objects list and tree */
static spinlock_t objects_lock;


/*** Locally used routines ***/

/* compare routine for objects AVL tree */
static int compare_object_id(const void *o1, const void *o2)
{
    if( ((vm_object_t *)o1)->id > ((vm_object_t *)o2)->id )
        return 1;
    if( ((vm_object_t *)o1)->id < ((vm_object_t *)o2)->id )
        return -1;
    return 0;
}

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

    /* put object into tree */
    if(!avl_tree_add(&objects_tree, object))
      panic("put_object_to_list(): failed to add object into tree!\n");

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
    xlist_remove_unsafe(&objects_list, &object->list_node);

    /* and remove it from tree */
    if(!avl_tree_remove(&objects_tree, object))
      panic("remove_object_from_list(): failed to remove object from tree!\n");

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

    /* init objects AVL tree */
    avl_tree_create( &objects_tree, compare_object_id,
                     sizeof(vm_object_t),
                     offsetof(vm_object_t, tree_node) );

    return NO_ERROR;
}
