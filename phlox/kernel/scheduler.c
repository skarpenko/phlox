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

/* look at head thread in priority queue */
static inline thread_t *rq_peek_head_thread(runqueue_t *rq, int prio)
{
    list_elem_t *tmp = xlist_peek_first(&rq->queue[prio]);
    return containerof(tmp, thread_t, sched_list_node);
}

/* look at next thread in priority queue */
static inline thread_t *rq_peek_next_thread(thread_t *th)
{
    list_elem_t *tmp = th->sched_list_node.next;
    return (tmp != NULL) ? containerof(tmp, thread_t, sched_list_node) : NULL;
}

/* pop head thread from run queue */
static inline thread_t *rq_pop_head_thread(runqueue_t *rq, int prio)
{
    list_elem_t *tmp = xlist_extract_first(&rq->queue[prio]);
    thread_t *th = containerof(tmp, thread_t, sched_list_node);
    rq->total_count--; /* decrease count of ready to run threads */
    return th;
}

/* extract thread from run queue */
static inline thread_t *rq_extract_thread(runqueue_t *rq, thread_t *th)
{
    /* unsafe version of remove used! */
    xlist_remove_unsafe(&rq->queue[th->d_prio], &th->sched_list_node);
    rq->total_count--; /* update ready to run threads count */
    return th;
}

/* choose more significant thread of two provided */
static thread_t *significant_thread(thread_t *t1, thread_t *t2)
{
    int pl1, pl2;

/* Note: Actually dynamic priorities comparsion is not needed,
 *       'cause two threads selected from the same queue.
*/
#if 0
    /* compare dynamic priorities */
    if(t1->d_prio > t2->d_prio)
        return t1;
    else if (t2->d_prio > t1->d_prio)
        return t2;
#endif

    /*** compare scheduling policies ***/

    /* get policy for fist thread */
    if( t1->sched_policy.policy.type == SCHED_POLICY_INTERACTIVE &&
       !(t1->process->flags & PROCESS_FLAG_INTERACTIVE) )
        pl1 = SCHED_POLICY_ORDINARY;
    else
        pl1 = t1->sched_policy.policy.type;

    /* get policy for second thread */
    if( t2->sched_policy.policy.type == SCHED_POLICY_INTERACTIVE &&
       !(t2->process->flags & PROCESS_FLAG_INTERACTIVE) )
        pl2 = SCHED_POLICY_ORDINARY;
    else
        pl2 = t2->sched_policy.policy.type;

    /* NOTE: SCHED_POLICY_INTERACTIVE policy is actual only if process has
     *       PROCESS_FLAG_INTERACTIVE flag set. If flag is not set
     *       then SCHED_POLICY_ORDINARY policy is used.
     */

    /* compare policies. if equal - return first thread */
    if (pl1 >= pl2)
        return t1;
    else
        return t2;
}

/* Carrots and Sticks policy implementation for thread */
static int carrots_and_sticks(thread_t *th)
{
    /* Arrays of penalties and bonuses for ordinary threads and
     * system threads. Index step equal to 10% step of time quanta usage.
     * So... If thread used all time quanta it receives penalty, if
     * only small piece used it receives bonus. Bonus/penalty added to
     * thread's dynamic priority.
     */
    static const int sys_bonus[10] = { 0, 0, 0, 0, 0, 0,  0,  0, -1, -1 };
    static const int ord_bonus[10] = { 3, 2, 2, 1, 0, 0, -1, -2, -3, -3 };
    const int n = sizeof(sys_bonus)/sizeof(int);
    const int *bonus;
    int ptyi;

    /* Select proper array.
     * Interrupt handling threads and hardware handling threads
     * uses special conditions.
     */
    if(th->sched_policy.policy.type == SCHED_POLICY_INTERRUPT ||
       th->sched_policy.policy.type == SCHED_POLICY_HW_HANDLER)
        bonus = sys_bonus;
    else
        bonus = ord_bonus;

    /* compute quanta usage percent */
    ptyi = 100 * (th->ijiffies - th->jiffies) / th->ijiffies / n;
    /* check resulting index */
    if(ptyi < 0) ptyi = 0;
    else if(ptyi >= n) ptyi = n-1;

    /* if thread is not system thread - compute resulting bonus or penalty */
    if(bonus == ord_bonus) {
        /* accumulate result */
        th->carrots_sticks += bonus[ptyi];
        /* if no bonus or penalty, reward or penalize thread
         * depending on current accumulated value of penalty/bonus.
        */
        if(bonus[ptyi] == 0) {
            if(th->carrots_sticks > 0) th->carrots_sticks--;
            else
            if(th->carrots_sticks < 0) th->carrots_sticks++;
        }

        /* check limits for penalty and bonus */
        if (th->carrots_sticks > SCHED_MAX_THREAD_BONUS)
            th->carrots_sticks = SCHED_MAX_THREAD_BONUS;
        else if (th->carrots_sticks < -SCHED_MAX_THREAD_PENALTY)
            th->carrots_sticks = -SCHED_MAX_THREAD_PENALTY;
    } else {
        /* system threads is not penalized hard */
        th->carrots_sticks = bonus[ptyi];
    }

    /* return result */
    return th->carrots_sticks;
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
