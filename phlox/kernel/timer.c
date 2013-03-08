/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <sys/debug.h>
#include <phlox/errors.h>
#include <phlox/processor.h>
#include <phlox/interrupt.h>
#include <phlox/thread.h>
#include <phlox/thread_private.h>
#include <phlox/scheduler.h>
#include <phlox/spinlock.h>
#include <phlox/avl_tree.h>
#include <phlox/list.h>
#include <phlox/heap.h>
#include <phlox/timer.h>


/* redefinition for convenience */
typedef xlist_t events_queue_t;


/* timer event type */
typedef struct event_struct {
    int delta;     /* timer ticks delta */

    bool timeout;  /* is timeout call? */
    union {
        /* valid if timeout is true */
        struct {
            timeout_id id;                             /* timeout call id */
            spinlock_t run_lock;                       /* running if locked */
            bool canceled;                             /* is canceled? */
            void (*timeout_routine)(timeout_id,void*); /* ptr to routine */
            void *data;                                /* user data */
        };
        /* valid if timeout is false */
        thread_t *thread;
    };
    list_elem_t list_node;
    avl_tree_node_t tree_node;
} event_t;


/* Timer ticks counter */
static volatile bigtime_t timer_ticks = 0;

/* Enumerator for timeout calls IDs */
static vuint next_timeout_id;

/* Timer events */
spinlock_t events_lock; /* access lock for queues below */
events_queue_t events_queue;
events_queue_t ready_timeouts_queue;

int events_ticks_lost = 0;

/* Timeout calls handler thread */
thread_t *timeout_calls_thread = NULL;

/* Tree of timeout calls for fast search by id */
static avl_tree_t timeouts_tree;
spinlock_t timeouts_lock;

/* called from from timer handler */
static bool timer_schedule_event(void);


/*** Locally used routines ***/

/* compare routine for timeouts AVL tree */
static int compare_timeout_id(const void *e1, const void *e2)
{
    ASSERT_MSG(((event_t *)e1)->timeout && ((event_t *)e2)->timeout,
        "compare_timeout_id(): not a timeout call!\n");

    if( ((event_t *)e1)->id > ((event_t *)e2)->id )
        return 1;
    if( ((event_t *)e1)->id < ((event_t *)e2)->id )
        return -1;
    return 0;
}

/* returns available timeout id */
static timeout_id get_next_timeout_id(void)
{
    timeout_id retval;

    /* atomically increment and get previous value */
    retval = (timeout_id)atomic_inc_ret((atomic_t*)&next_timeout_id);
    if(retval == INVALID_TIMEOUTID)
        panic("No available timeout IDs!");

    return retval;
}

/* returns pointer to next event */
static inline event_t *peek_next_event(event_t *e)
{
    if(!e || !xlist_peek_next(&e->list_node))
        return NULL;
    else
        return containerof(xlist_peek_next(&e->list_node), event_t, list_node);
}

/* returns pointer to first event */
static inline event_t *peek_first_event(void)
{
    list_elem_t *e = xlist_peek_first(&events_queue);
    if(!e)
        return NULL;
    else
        return containerof(e, event_t, list_node);
}

/* extract first event */
static inline event_t *extract_first_event(void)
{
    list_elem_t *e = xlist_extract_first(&events_queue);
    if(!e)
        return NULL;
    else
        return containerof(e, event_t, list_node);
}

/* extract and destroy first event */
static inline void destroy_first_event(void)
{
    list_elem_t *e = xlist_extract_first(&events_queue);
    if(!e)
        return;
    kfree( containerof(e, event_t, list_node) );
}

/* adds new event into queue */
static void add_new_event_nolock(event_t *new_evt, int tick)
{
    event_t *evt = peek_first_event();

    /* search for event to insert new event before */
    while(evt != NULL) {
        if(tick < evt->delta) {
            evt->delta -= tick;
            break;
        } else if(tick == evt->delta) {
            tick = 0;
            evt = peek_next_event(evt);
            break;
        } else {
            tick -= evt->delta;
        }
        evt = peek_next_event(evt);
    }

    new_evt->delta = tick; /* tick contains delta */

    /* add into queue */
    if(evt != NULL) {
       xlist_insert_before_unsafe(&events_queue, &evt->list_node, &new_evt->list_node);
    } else {
       xlist_add_last(&events_queue, &new_evt->list_node);
    }
}

/* extract timeout event from the appropriate queue */
static inline event_t *extract_timeout_event(void)
{
    list_elem_t *e = xlist_extract_first(&ready_timeouts_queue);
    if(!e)
        return NULL;
    else
        return containerof(e, event_t, list_node);
}

/* enqueue timeout event into appropriate queue */
static void enqueue_timeout_event(event_t *evt)
{
    xlist_add_last(&ready_timeouts_queue, &evt->list_node);
}

/* timeout calls handler thread routine */
static int timeout_calls_handler(void *data)
{
    unsigned long irqs_state;
    event_t *evt;

    while(1) {
         /* acquire events queue lock */
         irqs_state = spin_lock_irqsave(&events_lock);
         evt = extract_timeout_event();
         /* release events queue lock */
         spin_unlock_irqrstor(&events_lock, irqs_state);

         /* suspend if no events in queue, will be resumed
          * when new event be added.
          */
         if(!evt) {
             thread_suspend_current();
             continue;
         }

        ASSERT_MSG(evt->timeout, "timeout_calls_handler(): not a timeout call!\n");

        /* call routine only if event was not canceled */
        spin_lock(&evt->run_lock); /* set lock to mark running event */
        if(!evt->canceled) {
            evt->timeout_routine(evt->id, evt->data);
        }
        spin_unlock(&evt->run_lock); /* clear lock */

        /** Remove event data from tree and free it **/

        /* acquire events lock */
        irqs_state = spin_lock_irqsave(&timeouts_lock);

        /* remove from tree */
        if(!avl_tree_remove(&timeouts_tree, evt))
            panic("timeout_calls_handler(): failed to remove event from tree.");

        /* release events lock */
        spin_unlock_irqrstor(&timeouts_lock, irqs_state);

        kfree(evt); /* return memory to kernel */
    }

    return 0; /* control never goes here, but keep compiler happy */
}


/* system timer initialization */
status_t timer_init(kernel_args_t *kargs)
{
    status_t err;

    /* call architecture-specific init routine */
    err = arch_timer_init(kargs);
    if(err != NO_ERROR)
        return err;

    /* call platform-specific init routine */
    err = platform_timer_init(kargs);
    if(err != NO_ERROR)
        return err;

    /* init events data */
    spin_init(&events_lock);
    spin_init(&timeouts_lock);
    xlist_init(&events_queue);
    xlist_init(&ready_timeouts_queue);
    /* timeouts tree */
    avl_tree_create(&timeouts_tree, compare_timeout_id, sizeof(event_t),
                    offsetof(event_t, tree_node));

    /* next valid timeout id */
    next_timeout_id = 1;

    return NO_ERROR;
}

/* continue timer module initialization */
status_t timer_init_after_threading(kernel_args_t *kargs)
{
    /* create timeout calls handler thread */
    thread_id id = thread_create_kernel_thread("timeout_calls_handler_thread",
                        &timeout_calls_handler, NULL, false);
    if(id == INVALID_THREADID)
        return ERR_MT_GENERAL;

    /* get pointer to thread structure */
    timeout_calls_thread = thread_get_thread_struct(id);
    if(timeout_calls_thread == NULL)
        return ERR_MT_GENERAL;

    return NO_ERROR;
}

/* events queue handler */
static bool timer_schedule_event(void)
{
    int ticks;                     /* ticks to process */
    bool resched = false;          /* =true if reschedule required */
    bool timeouts_thread = false;  /* =true if timeout calls handler ready to run */
    event_t *evt;

    /* try to acquire access to events */
    if(!spin_trylock(&events_lock)) {
        ++events_ticks_lost;
        return resched;
    }

    ticks = events_ticks_lost + 1;
    events_ticks_lost = 0;

    /* get queue head */
    evt = peek_first_event();

    while(evt) {
        /* decrement delta ticks */
        if( (ticks = (evt->delta -= ticks)) <= 0 ) {
            /** time for event! **/

            if(!evt->timeout) {
                /* awake thread */
                thread_lock_thread(evt->thread);
                sched_add_thread(evt->thread);
                thread_unlock_thread(evt->thread);

                destroy_first_event(); /* destroy event data */
            } else {
                /* move event to timeout calls queue */
                extract_first_event();
                enqueue_timeout_event(evt);

                /* schedule timeouts handler for execution */
                if(!timeouts_thread) {
                    timeouts_thread = true;
                    /* lock thread first */
                    thread_lock_thread(timeout_calls_thread);
                    /* add to scheduler if was suspended */
                    if(timeout_calls_thread->state == THREAD_STATE_SUSPENDED)
                        sched_add_thread(timeout_calls_thread);
                    /* cancel suspended state on next reschedule */
                    if(timeout_calls_thread->next_state == THREAD_STATE_SUSPENDED)
                        timeout_calls_thread->next_state = THREAD_STATE_READY;
                    /* unlock thread */
                    thread_unlock_thread(timeout_calls_thread);
                }
            }

            /* update ticks */
            ticks = -ticks;
            resched = true;
        } else
            break;
        /* point to next event */
        evt = peek_first_event();
    }

    /* release access to events */
    spin_unlock(&events_lock);

    return resched;
}

/* timer tick event handler */
flags_t timer_tick(void)
{
    flags_t ret = INT_FLAGS_NOFLAGS;

    /* count tick */
    ++timer_ticks;

    /* process pending events */
    if(timer_schedule_event())
        ret |= INT_FLAGS_RESCHED;

    /* call scheduler */
    if(scheduler_timer())
        ret |= INT_FLAGS_RESCHED;

    return ret;
}

/* return ticks count */
bigtime_t timer_get_ticks(void)
{
    unsigned long irqs_state;
    bigtime_t ticks_cpy;

    /* disable interrupts and copy current ticks count to local var */
    local_irqs_save_and_disable(irqs_state);
    ticks_cpy = timer_ticks;
    /* restore interrupts and return ticks count to caller */
    local_irqs_restore(irqs_state);
    return ticks_cpy;
}

/* return milliseconds count */
bigtime_t timer_get_time(void)
{
    bigtime_t ticks = timer_get_ticks();
    return TIMER_TICKS_TO_MSEC(ticks);
}

/* lulls thread for a given count of timer ticks */
status_t timer_lull_thread(thread_t *thread, uint ticks)
{
    event_t *new_evt;
    unsigned long irqs_state;

    /*
     * Debug checks.
     * Thread must be locked before call and be in
     * READY or RUNNING state.
    */
    ASSERT_MSG(thread->lock != 0,
        "timer_lull_thread(): thread was not locked!\n");
    ASSERT_MSG(thread->state == THREAD_STATE_READY ||
        thread->state == THREAD_STATE_RUNNING,
        "timer_lull_thread(): thread in wrong state\n");
    ASSERT_MSG(thread->next_state == THREAD_STATE_READY ||
        thread->next_state == THREAD_STATE_RUNNING,
        "timer_lull_thread(): thread has wrong next state\n");
    /* Important note: Sequence of locks considered in this routine is
     * "thread lock" -> "events lock", but in routine where events
     * being processed, lock sequence reversed. Deadlock will never
     * occurs, because thread pointed by event is in SLEEPING state and
     * newly added thread here must never be in this state.
     * Anyway, be careful!
     */

    /* allocate memory for new event */
    new_evt = (event_t*)kmalloc(sizeof(event_t));
    if(!new_evt)
        panic("timer_lull_thread(): out of kernel heap!\n");

    /* set thread states */
    thread->next_state = THREAD_STATE_SLEEPING;
    /* if was taken directly from runqueue - update current state */
    if(IS_THREAD_STATE_READY(thread))
        thread->state = THREAD_STATE_SLEEPING;

    /* set event data */
    new_evt->thread = thread;
    new_evt->timeout = false; /* not a timeout call */

    /* acquire events lock */
    irqs_state = spin_lock_irqsave(&events_lock);

    /* add event to queue */
    add_new_event_nolock(new_evt, ticks);

    /* release events lock */
    spin_unlock_irqrstor(&events_lock, irqs_state);

    return NO_ERROR;
}

/* schedule timeout call */
timeout_id timer_timeout_sched(timeout_routine_t routine, void *data, uint ticks)
{
    event_t *new_evt;
    unsigned long irqs_state;

    /* allocate memory for new event */
    new_evt = (event_t*)kmalloc(sizeof(event_t));
    if(!new_evt)
         panic("timer_timeout_sched(): out of kernel heap!\n");

     /* set event data */
     new_evt->timeout = true;
     new_evt->id = get_next_timeout_id();
     new_evt->canceled = false;
     new_evt->timeout_routine = routine;
     new_evt->data = data;
     spin_init(&new_evt->run_lock);

     /* acquire events lock */
     irqs_state = spin_lock_irqsave(&events_lock);
     spin_lock(&timeouts_lock);

     /* add event to queue */
     add_new_event_nolock(new_evt, ticks);
     /* ...and to avl-tree */
     if(!avl_tree_add(&timeouts_tree, new_evt))
         panic("timer_timeout_sched(): failed to add event into tree!\n");;

     /* release events lock */
     spin_unlock(&timeouts_lock);
     spin_unlock_irqrstor(&events_lock, irqs_state);

     return new_evt->id; /* return event id */
}

/* cancel timeout call */
void timer_timeout_cancel(timeout_id tid)
{
    event_t *look_for, *evt;
    unsigned long irqs_state;

    look_for = containerof(&tid, event_t, id);

    /* acquire events lock */
    irqs_state = spin_lock_irqsave(&timeouts_lock);

    /* find event in tree and set canceled flag */
    evt = avl_tree_find(&timeouts_tree, look_for, NULL);
    if(evt)
        evt->canceled = true;

    /* release events lock */
    spin_unlock_irqrstor(&timeouts_lock, irqs_state);
}

/* cancel timeout call (synchronized) */
void timer_timeout_cancel_sync(timeout_id tid)
{
    event_t *look_for, *evt;
    unsigned long irqs_state;
    int retry;

    look_for = containerof(&tid, event_t, id);

    do {
        retry = 0; /* clear retry state */

        /* acquire events lock */
        irqs_state = spin_lock_irqsave(&timeouts_lock);

        /* find event in tree and set canceled flag */
        evt = avl_tree_find(&timeouts_tree, look_for, NULL);
        if(evt) {
            retry = !spin_trylock(&evt->run_lock);
            if(!retry) evt->canceled = true;
        }

        /* release events lock */
        spin_unlock_irqrstor(&timeouts_lock, irqs_state);

        /* Oops... Retry required. Wait a little. */
        if(retry)
            thread_yield();
    } while(retry);
}
