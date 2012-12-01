/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
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
#include <phlox/thread_private.h>
#include <phlox/thread.h>


/* Redefinition for convenience */
typedef xlist_t threads_list_t;

/* Next available thread id */
static vuint next_thread_id;

/* Threads list */
static threads_list_t threads_list;

/* Tree of threads for fast search by id */
static avl_tree_t threads_tree;

/* Dead threads list (for faster threads creation) */
static threads_list_t dead_threads_list;

/* Spinlock for operations on treads lists and tree */
static spinlock_t threads_lock;


/*** Locally used routines ***/

/* compare routine for threads AVL tree */
static int compare_thread_id(const void *t1, const void *t2)
{
    if( ((thread_t *)t1)->id > ((thread_t *)t2)->id )
        return 1;
    if( ((thread_t *)t1)->id < ((thread_t *)t2)->id )
        return -1;
    return 0;
}

/* returns available thread id */
static thread_id get_next_thread_id(void)
{
    thread_id retval;

    /* atomically increment and get previous value */
    retval = (thread_id)atomic_inc_ret(&next_thread_id);
    if(retval == INVALID_THREADID)
        panic("No available thread IDs!");
    /* TODO: Implement better approach for reliability. */

    return retval;
}

/* put thread to the end of threads list */
static void put_thread_to_list(thread_t *thread)
{
    unsigned long irqs_state;

    ASSERT_MSG(thread->state == THREAD_STATE_BIRTH, "Wrong thread state!");

    /* acquire lock before touching list */
    irqs_state = spin_lock_irqsave(&threads_lock);

    /* add item */
    xlist_add_last(&threads_list, &thread->threads_list_node);

    /* put thread into avl tree */
    if(!avl_tree_add(&threads_tree, thread))
      panic("put_thread_to_list(): failed to add thread into tree!\n");

    /* update thread state */
    thread->state = THREAD_STATE_BIRTH;

    /* release lock */
    spin_unlock_irqrstor(&threads_lock, irqs_state);
}

/* remove thread from list */
static void remove_thread_from_list(thread_t *thread)
{
    unsigned long irqs_state;

    ASSERT_MSG( thread->state == THREAD_STATE_BIRTH ||
                thread->state == THREAD_STATE_DEATH,
                "Wrong thread state!" );

    /* acquire lock before list access */
    irqs_state = spin_lock_irqsave(&threads_lock);

    /* remove thread */
    xlist_remove_unsafe(&threads_list, &thread->threads_list_node);

    /* remove thread from tree */
    if(!avl_tree_remove(&threads_tree, thread))
      panic("remove_thread_from_list(): failed to remove thread from tree!\n");

    /* release lock */
    spin_unlock_irqrstor(&threads_lock, irqs_state);
}

/* put thread into dead threads list */
static void put_thread_to_dead_list(thread_t *thread)
{
    unsigned long irqs_state;

    ASSERT_MSG( thread->state == THREAD_STATE_BIRTH ||
                thread->state == THREAD_STATE_DEATH,
                "Wrong thread state!" );

    /* acquire lock */
    irqs_state = spin_lock_irqsave(&threads_lock);

    /* add item */
    xlist_add_last(&dead_threads_list, &thread->threads_list_node);

    /* update thread state */
    thread->state = THREAD_STATE_DEAD;

    /* release lock */
    spin_unlock_irqrstor(&threads_lock, irqs_state);
}

/* extracts first item from dead threads list */
static thread_t *get_thread_from_dead_list(void)
{
    unsigned long irqs_state;
    list_elem_t *item;

    /* get lock */
    irqs_state = spin_lock_irqsave(&threads_lock);

    /* extract thread from list */
    item = xlist_extract_first(&dead_threads_list);

    /* release lock */
    spin_unlock_irqrstor(&threads_lock, irqs_state);

    /* return result */
    if(item)
      return containerof(item, thread_t, threads_list_node);
    else
      return NULL; /* list is empty */
}

