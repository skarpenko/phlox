/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <sys/debug.h>
#include <phlox/errors.h>
#include <phlox/thread.h>
#include <phlox/scheduler.h>
#include <phlox/scheduler_private.h>


/* Redefine for convenience */
typedef xlist_t threads_queue_t;

/* Idle threads for each cpu */
thread_t *idle_threads[SYSCFG_MAX_CPUS] = { 0 };

/* Tasks queue for scheduling */
static threads_queue_t threads_queue;

/* Spinlock for queues access */
static spinlock_t queues_lock;


/*** Locally used routines ***/

/* temporary routine. performs threads rotation in queue. */
static void do_round_robin(void)
{
    list_elem_t *item;

    /* extract first item */
    item = xlist_extract_first(&threads_queue);
    if(!item)
        return;

    /* add to tail of queue */
    xlist_add_last(&threads_queue, item);
}

/* peeks head of thread queue */
static thread_t *peek_threads_queue(void)
{
    list_elem_t *item;

    /* peek first item in queue */
    item = xlist_peek_first(&threads_queue);

    /* return result */
    if(item)
      return containerof(item, thread_t, sched_list_node);
    else
      return NULL; /* queue is empty */
}


/*** Public routines ***/

/* scheduler init */
status_t scheduler_init(kernel_args_t *kargs)
{
    status_t err;

    /* arch-specific init */
    err = arch_scheduler_init(kargs);
    if(err != NO_ERROR)
        return err;

    /* spinlock init */
    spin_init(&queues_lock);

    /* queue init */
    xlist_init(&threads_queue);

    return NO_ERROR;
}

/* scheduler per cpu init */
status_t scheduler_init_per_cpu(kernel_args_t *kargs, uint curr_cpu)
{
    status_t err;

    /* arch-specific per cpu init */
    err = arch_scheduler_init_per_cpu(kargs, curr_cpu);
    if(err != NO_ERROR)
        return err;

    return NO_ERROR;
}

/* called from timer tick handler */
bool scheduler_timer(void)
{
    thread_t *thread;

    /* current thread */
    thread = thread_get_current_thread();

    if(!thread)
        return false;

    /* jiffies counted down to zero
     * reschedule needed.
    */
    if(thread->jiffies == 0)
        return true;

    thread->jiffies--; /* throw one jiffy out */

    return false;
}

/* adds idle thread for cpu */
void sched_add_idle_thread(thread_t *thread, uint cpu)
{
    ASSERT_MSG(idle_threads[cpu] == 0,
        "sched_add_idle_thread(): idle thread already set!");

    /* set idle thread */
    thread->state = THREAD_STATE_READY;
    idle_threads[cpu] = thread;
}

/* add thread for scheduling */
void sched_add_thread(thread_t *thread)
{
    unsigned long irqs_state;

    ASSERT_MSG(thread->state == THREAD_STATE_BIRTH,
        "sched_add_thread(): thread in wrong state!");

    /* acquire lock first */
    irqs_state = spin_lock_irqsave(&queues_lock);

    /* set new state to thread */
    thread->state = THREAD_STATE_READY;

    /* add to queue */
    xlist_add_last(&threads_queue, &thread->sched_list_node);

    /* release lock */
    spin_unlock_irqrstor(&queues_lock, irqs_state);
}

/* reschedules and performs context switch */
void sched_reschedule(void)
{
    unsigned long irqs_state;
    thread_t *thread;
    thread_t *old_thread;

    old_thread = thread_get_current_thread();
    if(!old_thread)
        return;

    /* acquire lock */
    irqs_state = spin_lock_irqsave(&queues_lock);

    /* rotate queue */
    do_round_robin();

    /* release lock */
    spin_unlock_irqrstor(&queues_lock, irqs_state);

    /* peek queue head */
    thread = peek_threads_queue();
    if(!thread) return;

    /* set jiffies for next thread */
    thread->jiffies = DEFAULT_PER_THREAD_JIFFIES;

    /* switch to next thread */
    arch_sched_context_switch(old_thread, thread);
}
