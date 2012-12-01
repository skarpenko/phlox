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
#include <phlox/processor.h>
#include <phlox/scheduler.h>
#include <phlox/process.h>
#include <phlox/thread.h>
#include <phlox/thread_private.h>


/* Redefinition for convenience */
typedef xlist_t threads_list_t;

/* Thread lists types */
enum list_types {
    ALIVE_THREADS_LIST,  /* List of alive threads */
    DEAD_THREADS_LIST    /* List of dead threads */
};

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

/* put new thread into threads list (no lock acquired) */
static void put_thread_to_list_nolock(thread_t *thread)
{
    /* thread must be in Birth state */
    ASSERT_MSG(thread->state == THREAD_STATE_BIRTH,
        "put_thread_to_list_nolock(): Wrong thread state!");

    /* add thread to list */
    xlist_add_last(&threads_list, &thread->threads_list_node);

    /* put thread into avl tree */
    if(!avl_tree_add(&threads_tree, thread))
      panic("put_thread_to_list_nolock(): failed to add thread into tree!\n");
}

/* put new thread into threads list */
static void put_thread_to_list(thread_t *thread)
{
    unsigned long irqs_state;

    /* acquire lock before call to non-lock version */
    irqs_state = spin_lock_irqsave(&threads_lock);

    /* call non-lock version */
    put_thread_to_list_nolock(thread);

    /* release lock */
    spin_unlock_irqrstor(&threads_lock, irqs_state);
}

/* move thread from one list to another (non-lock version) */
static void move_thread_to_list_nolock(thread_t *thread, enum list_types dest_list)
{
    if(dest_list == ALIVE_THREADS_LIST) { /* Destination is alive threads list */
        /* Only dead threads can be moved to alive list */
        ASSERT_MSG(thread->state == THREAD_STATE_DEAD,
            "move_thread_to_list_nolock(): Wrong thread state!");

        /* remove thread from the deads */
        xlist_remove_unsafe(&dead_threads_list, &thread->threads_list_node);

        /* set birth state */
        thread->state = THREAD_STATE_BIRTH;

        /* put to alive threads list and tree */
        put_thread_to_list_nolock(thread);
    } else if(dest_list == DEAD_THREADS_LIST) { /* Destination is dead threads list */
        /* Only threads in Death and Birth state can be moved here */
        ASSERT_MSG(thread->state == THREAD_STATE_DEATH ||
                   thread->state == THREAD_STATE_BIRTH,
            "move_thread_to_list_nolock(): Wrong thread state!");

        /* remove thread from list */
        xlist_remove_unsafe(&threads_list, &thread->threads_list_node);
        /* remove thread from tree */
        if(!avl_tree_remove(&threads_tree, thread))
            panic("move_thread_to_list_nolock(): failed to remove thread from tree!\n");

        /* set as dead */
        thread->state = THREAD_STATE_DEAD;

        /* add to deads list */
        xlist_add_last(&dead_threads_list, &thread->threads_list_node);
    }
#ifdef __DEBUG__
    else
        panic("move_thread_to_list_nolock(): wrong list type passed!\n");
#endif
}

/* move thread from one list to another */
static void move_thread_to_list(thread_t *thread, enum list_types dest_list)
{
    unsigned long irqs_state;

    /* acquire lock before calling non-lock version */
    irqs_state = spin_lock_irqsave(&threads_lock);

    /* call to non-lock version */
    move_thread_to_list_nolock(thread, dest_list);

    /* release lock */
    spin_unlock_irqrstor(&threads_lock, irqs_state);
}

/* peeks the head of dead threads list (no lock acquired) */
static thread_t *peek_dead_list_nolock(void)
{
    list_elem_t *item;

    /* peek dead threads list */
    item = xlist_peek_first(&dead_threads_list);

    /* return result */
    if(item)
      return containerof(item, thread_t, threads_list_node);
    else
      return NULL; /* list is empty */
}

/* create new thread structure */
static thread_t *create_thread_struct(void)
{
    thread_t *thread;

    /* allocate memory for new tread struct */
    thread = (thread_t *)kmalloc(sizeof(thread_t));
    if(!thread)
        panic("create_thread_struct(): out of heap memory!\n");

    /* init fields */
    memset(thread, 0, sizeof(thread_t));
    thread->id = get_next_thread_id(); /* Thread ID */

    return thread;
}

/* get not used or create new thread structure */
static thread_t *get_thread_struct(void)
{
    unsigned long irqs_state;
    thread_t *thread;

    /* acquire lock before touching dead threads list */
    irqs_state = spin_lock_irqsave(&threads_lock);

    /* peek first item in list */
    thread = peek_dead_list_nolock();
    /* if exists - move to alive threads list */
    if(thread)
        move_thread_to_list_nolock(thread, ALIVE_THREADS_LIST);
    
    /* release lock */
    spin_unlock_irqrstor(&threads_lock, irqs_state);

    /* if no thread struct selected - create new one */
    if(!thread) {
        thread = create_thread_struct();
        thread->state = THREAD_STATE_BIRTH;
        put_thread_to_list(thread);
    }

    return thread;
}


/*** Public routines ***/

/* threading init routine */
status_t threading_init(kernel_args_t *kargs, uint curr_cpu)
{
    status_t err;

    if(curr_cpu == BOOTSTRAP_CPU) {
        /* Start threading initialization */

        /* arch-dependend init */
        err = arch_threading_init(kargs);
        if(err != NO_ERROR)
            return err;

        /* process module init */
        err = process_init(kargs);
        if(err != NO_ERROR)
            return err;

        /* scheduling engine init */
        err = scheduler_init(kargs);
        if(err != NO_ERROR)
            return err;

        /* next valid id is 1 */
        next_thread_id = 1;

        /* spinlock for data structures access */
        spin_init(&threads_lock);

        /* threads lists */
        xlist_init(&threads_list);
        xlist_init(&dead_threads_list);

        /* threads tree */
        avl_tree_create( &threads_tree, compare_thread_id,
                         sizeof(thread_t),
                         offsetof(thread_t, threads_tree_node) );
    } else {
      /* TODO: wait for bootstrap cpu completes */
    }

    /* start per CPU initializations */
    err = threading_init_per_cpu(kargs, curr_cpu);
    if(err != NO_ERROR)
        panic("threading_init: initialization for CPU #%d failed!\n", curr_cpu);

    return NO_ERROR;
}

/* per CPU threading init routine */
status_t threading_init_per_cpu(kernel_args_t *kargs, uint curr_cpu)
{
    status_t err;

    /* arch-dependend per cpu init */
    err = arch_threading_init_per_cpu(kargs, curr_cpu);
    if(err != NO_ERROR)
        return err;

    /* process module per cpu init */
    err = process_init_per_cpu(kargs, curr_cpu);
    if(err != NO_ERROR)
        return err;

    /* scheduler per cpu init */
    err = scheduler_init_per_cpu(kargs, curr_cpu);
    if(err != NO_ERROR)
        return err;

    return NO_ERROR;
}

/* returns current thread structure */
thread_t *thread_get_current_thread(void)
{
    /* pointer to thread struct stored in a bottom
     * of a thread kernel-side stack. So... Just get
     * a pointer to the bottom of current stack and
     * return it.
    */
    addr_t stack_ptr = arch_current_stack_pointer();
    return (thread_t *)(stack_ptr & (~(THREAD_KSTACK_SIZE-1)));
}

/* returns current thread id */
thread_id thread_get_current_thread_id(void)
{
    return thread_get_current_thread()->id;
}
