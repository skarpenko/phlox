/*
* Copyright 2007-2011, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <sys/debug.h>
#include <phlox/kernel.h>
#include <phlox/param.h>
#include <phlox/errors.h>
#include <phlox/processor.h>
#include <phlox/heap.h>
#include <phlox/list.h>
#include <phlox/atomic.h>
#include <phlox/avl_tree.h>
#include <phlox/spinlock.h>
#include <phlox/thread.h>
#include <phlox/thread_private.h>
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

/* Macro for creating value of semaphore id */
#define SEM_ID_BUILD(slot, gid) (((sem_id)gid)<<SEM_IDX_BITS | ((sem_id)slot))


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

/* Next generated id for semaphore */
static vuint next_sem_gid;

/* Semaphores tree (only for named semaphores) */
static avl_tree_t sem_tree;
/* Semaphores tree lock */
static spinlock_t sem_tree_lock;


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

/* compare routine for semaphores names */
static int compare_sem_name(const void *sem1, const void *sem2)
{
    return strcmp( ((semaphore_t*)sem1)->name, ((semaphore_t*)sem2)->name );
}

/* allocate and init semaphore structure */
static semaphore_t* create_sem_struct(const char *name)
{
    char *n = NULL;
    semaphore_t *sem;

    /* copy name string if exists */
    if(name != NULL && name[0]) {
        n = kstrdup(name);
        if(n == NULL)
            return NULL;
    }

    /* allocate memory for semaphore data */
    sem = (semaphore_t*)kmalloc(sizeof(semaphore_t));
    if(sem == NULL) {
        if(n != NULL) kfree(n);
        return NULL;
    }

    /* init fields */
    memset(sem, 0, sizeof(semaphore_t));
    sem->name = n;

    /* return to caller */
    return sem;
}

/* free memory occupied by semaphore data */
static void destroy_sem_struct(semaphore_t *sem)
{
    /* debug checks */
    ASSERT_MSG( xlist_isempty(&sem->waiters),
        "destroy_sem_struct(): wait list is not empty!" );

    /* free memory */
    if(sem->name) kfree(sem->name);
    kfree(sem);
}

/* allocate slot in semaphores table and set lock for it */
static int alloc_sem_slot(void)
{
    int i;
    for(i = 0; i < SYS_MAX_SEM_COUNT; ++i) {
        if(spin_trylock(&sem_table[i].lock)) {
            /* is free? */
            if(sem_table[i].sem == NULL)
                return i;
            /* not free, unlock and continue */
            spin_unlock(&sem_table[i].lock);
        }
    }
    return -1; /* no free slot */
}

/* return pointer to locked semaphore data */
static semaphore_t *get_sem_by_id(sem_id id)
{
    uint idx = SEM_IDX(id);
    semaphore_t *sem = NULL;

    /* check index is within semaphores table */
    if(idx >= SYS_MAX_SEM_COUNT)
        return NULL;

    /* lock item */
    spin_lock(&sem_table[idx].lock);

    /* check item is valid */
    if(sem_table[idx].sem != NULL && sem_table[idx].sem->id == id)
        sem = sem_table[idx].sem;
    else /* item not valid - unlock */
        spin_unlock(&sem_table[idx].lock);

    return sem; /* return to caller */
}

/* put control block into semaphore list */
static inline void sem_put_wcb(semaphore_t *sem, sem_wcb_t *wcb)
{
    xlist_add_last(&sem->waiters, &wcb->list_node);
}

/* extract control block from semaphore list */
static inline sem_wcb_t* sem_get_wcb(semaphore_t *sem)
{
    list_elem_t *e = xlist_extract_first(&sem->waiters);
    return e ? containerof(e, sem_wcb_t, list_node) : NULL;
}

/* peek head of the control blocks list */
static inline sem_wcb_t* sem_peek_first_wcb(semaphore_t *sem)
{
    list_elem_t *e = xlist_peek_first(&sem->waiters);
    return e ? containerof(e, sem_wcb_t, list_node) : NULL;
}

/* peek tail of the control blocks list */
static inline sem_wcb_t* sem_peek_last_wcb(semaphore_t *sem)
{
    list_elem_t *e = xlist_peek_last(&sem->waiters);
    return e ? containerof(e, sem_wcb_t, list_node) : NULL;
}

/* lock semaphore data */
static inline void sem_lock(semaphore_t *sem)
{
    spin_lock(sem->lock);
}

/* unlock semaphore data */
static inline void sem_unlock(semaphore_t *sem)
{
    spin_unlock(sem->lock);
}

/* called on thread death */
static void sem_thread_callback(thread_cbd_t *cb)
{
    sem_wcb_t *wcb = (sem_wcb_t*)cb->data;
    unsigned long irqs_state;

    /* remove thread control block from waiters list */
    irqs_state = spin_lock_irqsave(wcb->sem->lock);
    xlist_remove_unsafe(&wcb->sem->waiters, &wcb->list_node);
    spin_unlock_irqrstor(wcb->sem->lock, irqs_state);
}

/* timeout handler routine */
static void sem_timeout_callback(timeout_id id, void *data)
{
    sem_wcb_t *wcb = (sem_wcb_t*)data;
    unsigned long irqs_state;

    /* lock semaphore with interrupts disabled */
    irqs_state = spin_lock_irqsave(wcb->sem->lock);

    /* set wake up reason, if was not already set */
    if(wcb->err == NO_ERROR) wcb->err = ERR_SEM_TIMEOUT;
    /* remove control block from list */
    xlist_remove_unsafe(&wcb->sem->waiters, &wcb->list_node);

    /* wake up thread */
    thread_lock_thread(wcb->thread);
    sched_add_thread(wcb->thread);
    thread_unlock_thread(wcb->thread);

    /* unlock semaphore */
    spin_unlock_irqrstor(wcb->sem->lock, irqs_state);
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

    /* next available gid */
    next_sem_gid = 1;

    /* init AVL tree */
    avl_tree_create( &sem_tree,
                     compare_sem_name,
                     sizeof(semaphore_t),
                     offsetof(semaphore_t, tree_node) );

    /* init tree lock */
    spin_init(&sem_tree_lock);

    return NO_ERROR;
}

/* create semaphore (extended version) */
sem_id sem_create_ex(const char *name, uint max_count, uint init_count, proc_id owner)
{
    process_t *proc;
    uint has_name = (name != NULL && name[0]);
    unsigned long irqs_state;
    semaphore_t *sem;
    int slot;
    sem_id gid;

    /* check arguments */
    if((has_name && strlen(name) > SYS_MAX_OS_NAME_LEN) || init_count > max_count)
        return INVALID_SEMID;

    /* allocate slot */
    slot = alloc_sem_slot();
    if(slot < 0)
        return INVALID_SEMID;

    /* allocate semaphore structure */
    sem = create_sem_struct(name);
    if(sem == NULL) {
        spin_unlock(&sem_table[slot].lock);
        return INVALID_SEMID;
    }

    /* get owner process */
    proc = proc_get_process_by_id(owner);
    if(proc == NULL) {
        spin_unlock(&sem_table[slot].lock);
        destroy_sem_struct(sem);
        return INVALID_SEMID;
    }

    /* generate unique part of id */
    gid = get_next_gid(slot == 0);

    /* set fields */
    sem->id        = SEM_ID_BUILD(slot,gid);
    sem->lock      = &sem_table[slot].lock;
    sem->proc      = proc;
    sem->max_count = max_count;
    sem->count     = init_count;

    /* put semaphore data into table */
    sem_table[slot].sem = sem;

    /* disable local interrupts */
    local_irqs_save_and_disable(irqs_state);

    /* acquire locks */
    if(has_name) spin_lock(&sem_tree_lock);
    spin_lock(&proc->lock);

    /* put into tree */
    if(has_name && !avl_tree_add(&sem_tree, sem)) {
        /* Ooops... Semaphore with this name already exists. Revert! */
        /* unlock process and tree */
        spin_unlock(&proc->lock);
        spin_unlock(&sem_tree_lock);
        /* release semaphore data */
        sem_table[slot].sem = NULL;
        spin_unlock(&sem_table[slot].lock);
        destroy_sem_struct(sem);
        /* restore interrupts */
        local_irqs_restore(irqs_state);
        /* put process back */
        proc_put_process(proc);
        /* return error status */
        return INVALID_SEMID;
    }

    /* put into owned list of the process */
    xlist_add(&proc->semaphores, &sem->proc_list_node);

    /* release locks */
    spin_unlock(&proc->lock);
    if(has_name) spin_unlock(&sem_tree_lock);

    gid = sem->id; /* store return value */
    spin_unlock(&sem_table[slot].lock);

    /* restore interrupts */
    local_irqs_restore(irqs_state);

    /* put process back */
    proc_put_process(proc);

    /* return semaphore id to caller */
    return gid;
}

/* create semaphore */
sem_id sem_create(const char *name, uint max_count, uint init_count)
{
    return sem_create_ex( name, max_count, init_count, proc_get_current_process_id() );
}

/* delete semaphore */
status_t sem_delete(sem_id id)
{
    unsigned long irqs_state;
    semaphore_t *sem;
    sem_wcb_t *wcb;

    /* disable local interrupts */
    local_irqs_save_and_disable(irqs_state);

    /* get semaphore data */
    sem = get_sem_by_id(id);
    if(sem == NULL) {
        local_irqs_restore(irqs_state);
        return ERR_SEM_INVALID_HANDLE;
    }

    /* acquire locks */
    if(sem->name != NULL) spin_lock(&sem_tree_lock);
    spin_lock(&sem->proc->lock);

    /* remove from AVL tree */
    if(sem->name != NULL)
        if(!avl_tree_remove(&sem_tree, sem))
            panic("sem_delete(): failed to remove tree node!");

    /* remove from owned list of the process */
    xlist_remove_unsafe(&sem->proc->semaphores, &sem->proc_list_node);

    /* release locks */
    spin_unlock(&sem->proc->lock);
    if(sem->name != NULL) spin_unlock(&sem_tree_lock);

    /* free semaphores table slot */
    sem_table[SEM_IDX(id)].sem = NULL;
    sem_unlock(sem);

    /* wake up all waiting threads with DELETED error */
    while((wcb = sem_get_wcb(sem)) != NULL) {
        wcb->err = ERR_SEM_DELETED;
        /* wake up thread */
        thread_lock_thread(wcb->thread);
        sched_add_thread(wcb->thread);
        thread_unlock_thread(wcb->thread);
    }

    /* restore interrupts */
    local_irqs_restore(irqs_state);

    /* free memory occupied by semaphore data */
    destroy_sem_struct(sem);

    /* return status */
    return NO_ERROR;
}

/* count semaphore down (extended version) */
status_t sem_down_ex(sem_id id, uint count, uint timeout_msec, flags_t flags)
{
    unsigned long irqs_state;
    sem_wcb_t wcb;
    thread_cbd_t tcb;
    bool tcb_registered;

    /* disable local interrupts */
    local_irqs_save_and_disable(irqs_state);

    /* get semaphore data */
    wcb.sem = get_sem_by_id(id);
    { /* check for errors */
        status_t err = NO_ERROR;
        if(wcb.sem == NULL)
            err = ERR_SEM_INVALID_HANDLE;
        else if(wcb.sem->max_count < count)
            err = ERR_SEM_INVALID_VALUE;
        else if(!timeout_msec && (flags & SEMF_TIMEOUT))
            err = ERR_SEM_TIMEOUT;
        if(err != NO_ERROR || !count) {
            if(wcb.sem) sem_unlock(wcb.sem);
            local_irqs_restore(irqs_state);
            return err;
        }
    }

    /* store thread data in control block */
    wcb.thread = thread_get_current_thread_locked();
    wcb.count = count;

    /* set up thread callback data */
    tcb.thread = wcb.thread;
    tcb.func = &sem_thread_callback;
    tcb.data = &wcb;

    /* semaphore acquire loop */
    while(1) {
        /* reset vars */
        wcb.err = NO_ERROR;
        wcb.timeout = INVALID_TIMEOUTID;
        tcb_registered = false;

        /* can acquire? */
        if(wcb.sem->count >= count) {
            wcb.sem->count -= count;
            break;
        }

        /* if thread is going to run at next state - put it to wait state
         * or ignore for a while
         */
        if(IS_THREAD_NEXSTATE_READY(wcb.thread) || IS_THREAD_NEXSTATE_RUNNING(wcb.thread)) {
            /* if timeout wake up requested */
            if(flags & SEMF_TIMEOUT) {
                /* register timeout callback */
                wcb.timeout = timer_timeout_sched( sem_timeout_callback, &wcb,
                    TIMER_MSEC_TO_TICKS(timeout_msec) );
                /* check for errors */
                if(wcb.timeout == INVALID_TIMEOUTID) {
                    wcb.err = ERR_SEM_GENERAL;
                    break;
                }
            }

            /* set next state to WAITING and register callback */
            wcb.thread->next_state = THREAD_STATE_WAITING;
            thread_register_term_cb(&tcb);
            tcb_registered = true;

            /* put control block for thread into semaphore list */
            sem_put_wcb(wcb.sem, &wcb);
        }

        /* unlock thread and semaphore */
        thread_unlock_thread(wcb.thread);
        sem_unlock(wcb.sem);
        local_irqs_restore(irqs_state);

        /* reschedule */
        thread_yield();

        /* lock semaphore and thread after control returns here */
        local_irqs_save_and_disable(irqs_state);
        if(wcb.err != ERR_SEM_DELETED)  sem_lock(wcb.sem);  /* lock only if not deleted */
        thread_get_current_thread_locked();

        /* unregister callback if needed */
        if(tcb_registered)
            thread_unregister_term_cb(&tcb);

        /* unregister timeout call if needed */
        if(wcb.timeout != INVALID_TIMEOUTID)
            timer_timeout_cancel_sync(wcb.timeout);

        /* check for errors */
        if(wcb.err != NO_ERROR)
            break;
    }

    /* unlock thread and semaphore */
    thread_unlock_thread(wcb.thread);
    if(wcb.err != ERR_SEM_DELETED)  sem_unlock(wcb.sem);  /* unlock only if not deleted */
    local_irqs_restore(irqs_state);

    /* return state to caller */
    return wcb.err;
}

/* count semaphore down */
status_t sem_down(sem_id id, uint count)
{
    return sem_down_ex(id, count, 0, SEMF_NOFLAGS);
}

/* count semaphore up */
status_t sem_up(sem_id id, uint count)
{
    unsigned long irqs_state;
    semaphore_t *sem;
    sem_wcb_t *wcb, *wcb_f;
    uint curr_count;

    /* disable local interrupts */
    local_irqs_save_and_disable(irqs_state);

    /* get semaphore data */
    sem = get_sem_by_id(id);
    { /* check for errors */
        status_t err = NO_ERROR;
        if(sem == NULL)
            err = ERR_SEM_INVALID_HANDLE;
        else if(sem->count + count > sem->max_count)
            err = ERR_SEM_INVALID_VALUE;

        if(err != NO_ERROR || !count) {
            if(sem) sem_unlock(sem);
            local_irqs_restore(irqs_state);
            return err;
        }
    }

    /* update semaphore count */
    sem->count += count;
    curr_count = sem->count;

    /* get last item of control blocks list to process */
    wcb_f = sem_peek_last_wcb(sem);
    ASSERT_MSG(wcb_f != NULL, "sem_up(): control blocks list is empty!");

    /* walk through control blocks list to find threads for wake up */
    do {
        wcb = sem_get_wcb(sem);  /* get block from head */
        /* can acquire? */
        if(wcb->count <= curr_count) {
            /* wake up thread */
            thread_lock_thread(wcb->thread);
            sched_add_thread(wcb->thread);
            thread_unlock_thread(wcb->thread);

            /* update current count value */
            curr_count -= wcb->count;
        }
        sem_put_wcb(sem, wcb);  /* put block to tail */
    } while(wcb != wcb_f);

    /* unlock semaphore */
    sem_unlock(sem);
    local_irqs_restore(irqs_state);

    return NO_ERROR;
}

/* find semaphore by its name */
sem_id sem_get_by_name(const char *name)
{
    unsigned long irqs_state;
    sem_id id = INVALID_SEMID;
    semaphore_t *search4, *sem;

    /* search cookie for AVL-tree */
    search4 = containerof(&name, semaphore_t, name);

    /* lock tree with interrupts disabled */
    irqs_state = spin_lock_irqsave(&sem_tree_lock);

    /* search for semaphore */
    sem = avl_tree_find(&sem_tree, search4, NULL);
    if(sem) id = sem->id;

    /* unlock tree */
    spin_unlock_irqrstor(&sem_tree_lock, irqs_state);

    return id; /* return result */
}
