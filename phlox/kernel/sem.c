/*
* Copyright 2007-2011, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <sys/debug.h>
#include <phlox/kernel.h>
#include <phlox/param.h>
#include <phlox/errors.h>
#include <phlox/heap.h>
#include <phlox/list.h>
#include <phlox/atomic.h>
#include <phlox/avl_tree.h>
#include <phlox/spinlock.h>
#include <phlox/simple_lock.h>
#include <phlox/thread.h>
#include <phlox/process.h>
#include <phlox/timer.h>
#include <phlox/sem.h>


/**************************************************
 * Macro definitions
 **************************************************/

#if SYS_MAX_SEM_COUNT == 1024
#  define SEM_IDX_BITS 10
#elif SYS_MAX_SEM_COUNT == 2048
#  define SEM_IDX_BITS 11
#elif SYS_MAX_SEM_COUNT == 4096
#  define SEM_IDX_BITS 12
#elif SYS_MAX_SEM_COUNT == 8192
#  define SEM_IDX_BITS 13
#elif SYS_MAX_SEM_COUNT == 16384
#  define SEM_IDX_BITS 14
#else
#  error Unsupported value of SYS_MAX_SEM_COUNT!
#endif

/* Mask for semaphore index within semaphores table */
#define SEM_IDX_MASK  ((1<<SEM_IDX_BITS)-1)
/* Mask for semaphore generated id */
#define SEM_GID_MASK  (((sem_id)(-1))<<SEM_IDX_BITS)

/* Macroses for getting index and generated id from sem_id type */
#define SEM_IDX(a) ((a)&SEM_IDX_MASK)
#define SEM_GID(a) (((a)&SEM_GID_MASK)>>SEM_IDX_BITS)



/**************************************************
 * Semaphores types definitions
 **************************************************/


/* Semapore description structure */
typedef struct {
    sem_id          id;              /* semaphore id */
    spinlock_t      *lock;           /* pointer to semaphore slot access lock */
    process_t       *proc;           /* owner process */
    char            *name;           /* semaphore name (can be NULL) */
    uint            max_count;       /* maximum count for semaphore */
    uint            count;           /* current count */
    xlist_t         waiters;         /* list of waiting threads */
    list_elem_t     proc_list_node;  /* list node inside list of owner process */
    avl_tree_node_t tree_node;       /* tree node for search by name */
} semaphore_t;

/* Control block for waiting thread */
typedef struct {
    semaphore_t *sem;       /* owner semaphore */
    thread_t    *thread;    /* waiting thread */
    uint        count;      /* count requested by thread */
    timeout_id  timeout;    /* timeout call id */
    status_t    err;        /* wakeup status to return to thread */
    list_elem_t list_node;  /* list node inside waiters list of semaphore */
} sem_wcb_t;

/* Semaphore item within semaphores table */
typedef struct {
    spinlock_t  lock;  /* item lock */
    semaphore_t *sem;  /* pointer to semaphore if allocated or NULL */
} sem_table_item_t;


/**************************************************
 * Internally used data structures
 **************************************************/

/* Semaphores table */
static sem_table_item_t sem_table[SYS_MAX_SEM_COUNT];
/* Semaphores table lock (acquired by slot allocator only) */
static simple_lock_t sem_table_lock;

/* Next generated id for semaphore */
static vuint next_sem_gid;

/* Semaphores tree (only for named semaphores) */
static avl_tree_t sem_tree;
/* Semaphores tree lock */
static simple_lock_t sem_tree_lock;


/**************************************************
 * Internally used routines
 **************************************************/

/* return next available gid */
static sem_id get_next_gid(bool nozero)
{
    sem_id retval;
    do {
        retval = (sem_id)atomic_inc_ret(&next_sem_gid);
        if(retval > SEM_GID((sem_id)(-1))) {
            /* next_sem_id update here is safe, because
             * gid can be non-unique.
             */
            retval = 0;
            atomic_set(&next_sem_gid, retval);
        }
    } while(nozero && !retval);

    return retval;
}


/**************************************************
 * Public routines
 **************************************************/

 
/* init semaphores */
status_t semaphores_init(kernel_args_t *kargs)
{
    uint i;

    /* init semaphores table */
    for(i = 0; i < SYS_MAX_SEM_COUNT; ++i) {
        spin_init(&sem_table[i].lock);  /* init lock */
        sem_table[i].sem = NULL;        /* pointer to semaphore */
    }

    /* init table lock */
    simple_lock_init(&sem_table_lock);

    /* next available gid */
    next_sem_gid = 1;

    /* init tree */
/*    sem_tree;*/

    /* init tree lock */
    simple_lock_init(&sem_tree_lock);

    return NO_ERROR;
}


