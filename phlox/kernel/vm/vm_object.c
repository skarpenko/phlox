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
#include <phlox/vm_private.h>
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

/* compare routine for universal pages tree */
static int compare_upage(const void *up1, const void *up2)
{
    if( ((vm_upage_t *)up1)->upn > ((vm_upage_t *)up2)->upn )
        return 1;
    if( ((vm_upage_t *)up1)->upn < ((vm_upage_t *)up2)->upn )
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

/* common routine for creating virtual memory objects */
static vm_object_t *create_object_common(const char *name, size_t size, uint protection)
{
    vm_object_t *object;

    /* allocate object structure */
    object = (vm_object_t *)kmalloc(sizeof(vm_object_t));
    if(!object)
        return NULL;

    /* if creating named object - copy its name into structure field */
    if(name) {
        object->name = kstrdup(name);
        if(!object->name)
            goto error;
    } else {
        object->name = NULL;
    }

    /* init object structure fields */
    spin_init(&object->lock);
    object->size = PAGE_ALIGN(size);
    object->state = VM_OBJECT_STATE_NORMAL;
    object->protect = protection;
    object->flags = 0; /* currently not used */
    object->ref_count = 0;

    /* init bookkeeping structures */
    xlist_init(&object->mappings_list);
    xlist_init(&object->upages_list);
    avl_tree_create( &object->upages_tree, compare_upage,
                     sizeof(vm_upage_t),
                     offsetof(vm_upage_t, tree_node) );

    /* assign object id */
    object->id = get_next_object_id();

    /* return result to caller */
    return object;

error:
    /* release memory on error */
    if(object->name)
        kfree(object->name);
    kfree(object);

    return NULL; /* failed to create object */
}


/*** Public routines ***/

/* Module initialization routine */
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

/* create virtual memory object */
object_id vm_create_object(const char *name, size_t size, uint protection)
{
    vm_object_t *object;
    object_id id;

    /* create object */
    object = create_object_common(name, size, protection);
    if(!object)
        return VM_INVALID_OBJECTID;

    id = object->id;

    /* add to objects list */
    put_object_to_list(object);

    return id; /* return id */
}

/* returns object by its id */
vm_object_t *vm_get_object_by_id(object_id oid)
{
    vm_object_t temp_object, *object;
    unsigned long irqs_state;

    temp_object.id = oid;

    /* acquire lock before accessing tree */
    irqs_state = spin_lock_irqsave(&objects_lock);

    /* search tree */
    object = avl_tree_find(&objects_tree, &temp_object, NULL);

    /* release lock */
    spin_unlock_irqrstor(&objects_lock, irqs_state);

    /* if object found - increase references count */
    if(object)
        atomic_inc(&object->ref_count);

    return object;
}

/* put previously taken object */
void vm_put_object(vm_object_t *object)
{
    /* decrease references count */
    atomic_dec(&object->ref_count);
    /* TODO: implement additional functionality */
}

/* returns object id by its name */
object_id vm_find_object_by_name(const char *name)
{
    unsigned long irqs_state;
    list_elem_t *item;
    vm_object_t *object;
    object_id id = VM_INVALID_OBJECTID;

    /* return invalid id if NULL passed */
    if(!name)
        return VM_INVALID_OBJECTID;

    /* acquire lock before touching list */
    irqs_state = spin_lock_irqsave(&objects_lock);

    /* start search from first list item */
    item = xlist_peek_first(&objects_list);
    while(item) {
        /* convert node to object structure */
        object = containerof(item, vm_object_t, list_node);

        /* we skip unnamed and in deletion state */
        if(object->state != VM_OBJECT_STATE_DELETION &&
           object->name && !strcmp(object->name, name))
        {
            id = object->id;
            break; /* search successful */
        }

        /* step to next item */
        item = xlist_peek_next(item);
    }

    /* release lock */
    spin_unlock_irqrstor(&objects_lock, irqs_state);

    return id;
}
