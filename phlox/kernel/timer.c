/*
* Copyright 2007-2010, Stepan V.Karpenko. All rights reserved.
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
#include <phlox/list.h>
#include <phlox/heap.h>
#include <phlox/timer.h>


/* redefinition for convenience */
typedef xlist_t events_queue_t;

/* timer event type */
typedef struct event_struct {
    int delta;
    thread_t *thread;
    list_elem_t list_node;
} event_t;


/* Timer ticks counter */
static volatile bigtime_t timer_ticks = 0;

/* Timer events */
spinlock_t events_lock;
events_queue_t events_queue;
int events_ticks_lost = 0;


/* called from from timer handler */
static bool timer_schedule_event(void);


/*** Locally used routines ***/

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
    xlist_init(&events_queue);

    return NO_ERROR;
}

/* events queue handler */
static bool timer_schedule_event(void)
{
    int ticks;              /* ticks to process */
    bool resched = false;   /* =true if reschedule required */
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

            /* awake thread */
            thread_lock_thread(evt->thread);
            sched_add_thread(evt->thread);
            thread_unlock_thread(evt->thread);

            destroy_first_event(); /* destroy event data */

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

     /* acquire events lock */
     irqs_state = spin_lock_irqsave(&events_lock);

     /* set thread states */
     thread->next_state = THREAD_STATE_SLEEPING;
     /* if was taken directly from runqueue - update current state */
     if(IS_THREAD_STATE_READY(thread))
         thread->state = THREAD_STATE_SLEEPING;

     /* set event data */
     new_evt->thread = thread;

     /* add event to queue */
     add_new_event_nolock(new_evt, ticks);

     /* release events lock */
     spin_unlock_irqrstor(&events_lock, irqs_state);

     return NO_ERROR;
}
