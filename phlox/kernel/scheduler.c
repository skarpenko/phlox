/*
* Copyright 2007-2010, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <sys/debug.h>
#include <phlox/errors.h>
#include <phlox/thread.h>
#include <phlox/scheduler.h>
#include <phlox/scheduler_private.h>


/* Scheduling timer state */
static bigtime_t sched_ticks      = 0;      /* timer ticks counted */
static bool      in_sched_tick    = false;  /* =true if in timer handler already */
static vint      sched_lost_ticks = 0;      /* used for counting lost ticks */
static int       sched_tick_type  = 0;      /* current tick type */

/* Scheduler parameters */
static int quanta = THREAD_DEFAULT_QUANTA;
static uint shuffle_factor = THREAD_DEFAULT_QUANTA / SCHED_FACTOR_BMAX;

/* Per cpu run queues */
static runqueue_t runqueues[SYSCFG_MAX_CPUS];

/* Idle threads for each cpu */
static thread_t *idle_threads[SYSCFG_MAX_CPUS] = { NULL };


/*** Locally used routines ***/

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

/* put thread to proper priority queue.
 * Note: thread must not be contained in other scheduler structures!
 */
static void rq_put_thread(runqueue_t *rq, thread_t *th)
{
    /* affiliate thread to proper cpu */
    if(th->cpu == NULL || th->cpu->cpu_num != rq->cpu_num)
        th->cpu = &ProcessorSet.processors[rq->cpu_num];

    ASSERT_MSG(th->state == THREAD_STATE_READY,
        "rq_put_thread(): wrong thread state!");
    ASSERT_MSG(th->sched_list_node.next == NULL &&
        th->sched_list_node.prev == NULL,
        "rq_put_thread(): thread already assigned to list!");

   /* put to specified runqueue */
   xlist_add_last(&rq->queue[th->d_prio], &th->sched_list_node);
   rq->total_count++; /* update ready to run threads count */
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

/* shuffle runqueue head depending on tick type and threads wait time */
static void sched_shuffle_runqueue_head(runqueue_t *rq, int tick_type)
{
    thread_t *th;
    int queue_bonus;
    int prio;
    int new_prio;
    bigtime_t ticks_msec = SCHED_TICKS2MSEC(sched_ticks);

    /* walk through head of thread queues and recalculate
     * dynamic priorities for head threads depending on
     * time spent in waiting for execution.
     */
    for ( prio = tick_type; prio < THREAD_NUM_PRIORITY_LEVELS - 1;
          prio += SCHED_TICK_TYPES_COUNT )
    {
        /* skip empty queues */
        if(!rq->queue[prio].count) continue;

        /* point to head thread */
        th = rq_peek_head_thread(rq, prio);

        /* NOTE: Actually it is not needed to skip locked threads,
         * because the only thing touched here is a dynamic priority
         * which is managed only by scheduler and must not be touched
         * outside.
        */
        /* try to lock thread */
        /* if(!thread_trylock_thread(th))
         *     continue;
        */

        /* calculate bonus for queue */
        queue_bonus = rq->queue[prio].count >> SCHED_QUEUE_BONUS_THRESHOLD2;

        /* calculate new dynamic priority for thread */
        new_prio = th->s_prio + ((ticks_msec - th->sched_stamp) / shuffle_factor) +
                   queue_bonus + th->carrots_sticks;
        if(new_prio > THREAD_NUM_PRIORITY_LEVELS - 1)
            new_prio = THREAD_NUM_PRIORITY_LEVELS - 1;

        /* if new priority for thread is greater than current
         * dynamic priority - move thread to higher queue.
         */
        if(new_prio > th->d_prio) {
            /* remove thread from current queue */
            rq_pop_head_thread(rq, prio);
            /* set new dynamic priority */
            th->d_prio = new_prio;
            /* put thread back to proper queue */
            rq_put_thread(rq, th);
        }

        /* NOTE: (see above)
        */
        /* unlock thread */
        /* thread_unlock_thread(th); */
    }
}

/* init runqueue before first use */
static void sched_init_runqueue(runqueue_t *rq)
{
    int i;

    /* init spinlock */
    spin_init(&rq->lock);

    /* set vars */
    rq->total_count = 0;
    rq->cpu_num = 0;

    /* init priority queues */
    for(i=0; i < THREAD_NUM_PRIORITY_LEVELS; i++)
        xlist_init(&rq->queue[i]);
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

    /* TODO: do something? */

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

    /* init runqueue for this cpu */
    sched_init_runqueue(&runqueues[curr_cpu]);
    runqueues[curr_cpu].cpu_num = curr_cpu;

    /* ok */
    return NO_ERROR;
}

/* called from timer tick handler */
bool scheduler_timer(void)
{
    bool resched = false; /* return value */
    uint irqs_state;
    int ticks_out;
    int cpu;
    int prio;
    bool is_idle;
    runqueue_t *rq;
    thread_t *run_th;

    /* if nested enter occurs - increase lost ticks and exit */
    if(in_sched_tick) {
        sched_lost_ticks++;
        return false;
    } else
        in_sched_tick = true;

    /* calc scheduler ticks and ticks out value for current thread */
    sched_ticks += (ticks_out = sched_lost_ticks + 1);
    sched_lost_ticks = 0; /* reset lost ticks */

    /* current cpu, runqueue and thread */
    cpu = get_current_processor();
    rq = &runqueues[cpu];
    run_th = thread_get_current_thread();

    /* lock access to runqueue */
    irqs_state = spin_lock_irqsave(&rq->lock);

    /* set scheduler tick type and shuffle runqueue head */
    sched_tick_type = (sched_tick_type + 1) & SCHED_TICK_TYPES_MASK;
    sched_shuffle_runqueue_head(rq, sched_tick_type);

    /* if current tick is estimation tick, we need
     * estimate new value of quanta and shuffle factor.
     */
    if(sched_tick_type == SCHED_ESTIMATION_TICK) {
        uint a, b;

        /* estimate quanta */
        quanta = THREAD_DEFAULT_QUANTA - (rq->total_count >> SCHED_QUANTA_THRESHOLD2);
        if (quanta < THREAD_MINIMUM_QUANTA) quanta = THREAD_MINIMUM_QUANTA;

        /* estimate a and b params of shuffle factor */
        a = rq->total_count / SCHED_FACTOR_A; if(a==0) a = 1;
        b = (!rq->total_count) ? SCHED_FACTOR_BMAX : SCHED_FACTOR_BTHR / rq->total_count;
        if (b > SCHED_FACTOR_BMAX) b = SCHED_FACTOR_BMAX; if(b==0) b = 1;
        /* compute shuffle factor */
        shuffle_factor = a * SCHED_FACTOR_BASE / b;
    }

    /* if it is possible to reschedule current thread, try to find better one */
    if( !run_th->preempt_count && !(run_th->flags & THREAD_FLAG_RESCHEDULE) ) {
        /* is current thread is idle thread ? */
        is_idle = (idle_threads[cpu] == run_th);

        /* reschedule if not idle thread and jiffies is out */
        if( !is_idle && (((run_th->jiffies -= ticks_out) <= 0) ? run_th->jiffies = 0, true : false) )
            resched = true;
        else if ( is_idle || (SCHED_TICKS2MSEC(run_th->ijiffies - run_th->jiffies) >= THREAD_QUANTA_GRANULARITY) ) {
            /* find higher priority thread if current thread is idle thread or quanta
             * granule value is exceeded.
             */
            for(prio=THREAD_NUM_PRIORITY_LEVELS - 1; prio > run_th->d_prio; --prio)
                if (rq->queue[prio].count) {
                    resched = true; /* bingo! thread founded! */
                    break;
                }
        }

        /* mark thread as being rescheduled if rescheduling is needed */
        if(resched)
            run_th->flags |= THREAD_FLAG_RESCHEDULE;
    }

    /* unlock runqueue access */
    spin_unlock_irqrstor(&rq->lock, irqs_state);

    in_sched_tick = false; /* we leave scheduler's timer handler */

    /* return result */
    return resched;
}

/* adds idle thread for cpu */
void sched_add_idle_thread(thread_t *thread, uint cpu)
{
    ASSERT_MSG(idle_threads[cpu] == 0,
        "sched_add_idle_thread(): idle thread already set!");

    /* set idle thread */
    thread->state = THREAD_STATE_RUNNING; /* idle threads adds theirselves,
                                           * so... running state is correct.
                                           */
    thread->next_state = THREAD_STATE_READY;
    thread->s_prio = thread->d_prio = -1; /* idle threads never exists in runqueues */
    /* set thread */
    idle_threads[cpu] = thread;
}

/* add thread for scheduling */
void sched_add_thread(thread_t *thread)
{
    unsigned long irqs_state;
    runqueue_t *rq;

    ASSERT_MSG(thread->lock != 0,
        "sched_add_thread(): thread was not locked before!");

    ASSERT_MSG(thread->state == THREAD_STATE_BIRTH ||
               thread->state == THREAD_STATE_WAITING ||
               thread->state == THREAD_STATE_SUSPENDED,
        "sched_add_thread(): thread in wrong state!");

    /* check static priority value */
    if(thread->s_prio > THREAD_NUM_PRIORITY_LEVELS-1)
        thread->s_prio = THREAD_NUM_PRIORITY_LEVELS-1;

    /* get runqueue for add thread. thread added to runqueue
     * of current processor, it might be not good in SMP config.
     * so, fix this in fututre.
     */
    rq = &runqueues[ get_current_processor() ];

    /* set thread fields before */
    thread->state = THREAD_STATE_READY;
    thread->next_state = THREAD_STATE_RUNNING;
    thread->d_prio = thread->s_prio;
    thread->sched_stamp = SCHED_TICKS2MSEC(sched_ticks);

    /* acquire lock for runqueue */
    irqs_state = spin_lock_irqsave(&rq->lock);

    /* add to runqueue */
    rq_put_thread(rq, thread);

    /* release lock */
    spin_unlock_irqrstor(&rq->lock, irqs_state);
}

/* remove thread from scheduling */
void sched_remove_thread(thread_t *thread)
{
   /* NOTE: must be in death or dead states? */
   /* NOTE2: this call might be not needed */
}

/* capture cpu by current thread */
int sched_capture_cpu(void)
{
    thread_t *t = thread_get_current_thread();
    t->preempt_count++;
    return t->preempt_count;
}

/* release cpu captured by current thread */
int sched_release_cpu(void)
{
    thread_t *t = thread_get_current_thread();
    int tmp = t->preempt_count - 1;
    if (tmp<0) tmp = 0;
    t->preempt_count = tmp;
    return t->preempt_count;
}

/* performs last steps of context switch */
void sched_complete_context_switch(void)
{
    /* unlock current thread */
    thread_unlock_thread( thread_get_current_thread() );

    /* enable interrupts */
    local_irqs_enable();
}

/* reschedules and performs context switch */
void sched_reschedule(void)
{
    int cpu;
    int prio;
    bool is_idle;
    runqueue_t *rq;
    thread_t *curr_thrd, *next_thrd;

    /* disable interrupts */
    local_irqs_disable();

    /* init some other important variables */
    cpu = get_current_processor();                  /* current cpu */
    curr_thrd = thread_get_current_thread_locked(); /* current thread */
    next_thrd = NULL;                               /* next thread is unknown at this point */
    is_idle = (idle_threads[cpu] == curr_thrd);     /* is current thread is a per cpu idle thread? */

    /* acquire lock for runqueue and start */
    rq = &runqueues[cpu];                       /* runqueue for this cpu */
    spin_lock(&rq->lock);                       /* get lock */

    /* adjust current thread */
    curr_thrd->sched_stamp = SCHED_TICKS2MSEC(sched_ticks);  /* put time stamp */
    curr_thrd->flags &= ~THREAD_FLAG_RESCHEDULE;             /* reset RESCHEDULE flag if set */
    curr_thrd->state = curr_thrd->next_state;                /* switch to next state */
    curr_thrd->next_state = THREAD_STATE_READY;              /* reset next state */

    /* analyze new thread state */
    if(!is_idle)
      switch(curr_thrd->state) {
        /* continue of execution requested for this thread */
        case THREAD_STATE_RUNNING:
            /* init jiffies again */
            curr_thrd->jiffies = curr_thrd->ijiffies = SCHED_MSEC2TICKS(quanta);

            /* unlock runqueue, thread and return */
            spin_unlock(&rq->lock);
            thread_unlock_thread(curr_thrd);
            local_irqs_enable();
            return;

        /* put thread back to runqueue */
        case THREAD_STATE_READY:
            /* compute new dynamic priority */
            curr_thrd->d_prio = curr_thrd->s_prio + carrots_and_sticks(curr_thrd);

            /* ensure resulting dynamic priority value is correct */
            if(curr_thrd->d_prio > THREAD_NUM_PRIORITY_LEVELS-1)
                curr_thrd->d_prio = THREAD_NUM_PRIORITY_LEVELS-1;
            else if(curr_thrd->d_prio < THREAD_LOWEST_PIORITY)
                curr_thrd->d_prio = THREAD_LOWEST_PIORITY;

            /* put back to runqueue */
            rq_put_thread(rq, curr_thrd);
            break;

        /* do not put thread back to runqueue */
        case THREAD_STATE_WAITING:
        case THREAD_STATE_SUSPENDED:
        case THREAD_STATE_DEATH:
            break;

        default:
            panic("sched_reschedule(): invalid thread state!");
    }

    /* unlock current thread */
    thread_unlock_thread(curr_thrd);

    /* search priority queues for better thread */
    for(prio=THREAD_NUM_PRIORITY_LEVELS-1; prio>=0; prio--) {
        /* start new iteration if current queue is empty */
        if(rq->queue[prio].count == 0) continue;

        /* if more then 1 ready thread exists look for better one */
        if(rq->queue[prio].count > 1) {
            thread_t *tmp_next;  /* temp pointer for walking throught queue */
            uint n = SCHED_QUEUE_LOOKAHEAD_DEPTH; /* walk depth */

            /* find most significant thread in queue */
            next_thrd = rq_peek_head_thread(rq, prio);
            tmp_next = next_thrd;
            while( (tmp_next = rq_peek_next_thread(tmp_next)) != NULL && --n )
                next_thrd = significant_thread(next_thrd, tmp_next);

            /* extract discovered thread from queue */
            rq_extract_thread(rq, next_thrd);
        }
        else
            next_thrd = rq_pop_head_thread(rq, prio); /* select thread */

        /* cancel search cycle, we found something */
        break;
    }

    /* unlock runqueue */
    spin_unlock(&rq->lock);

    /* if thread was found */
    if(next_thrd != NULL)
        /* init jiffies for new thread */
        next_thrd->jiffies = next_thrd->ijiffies = SCHED_MSEC2TICKS(quanta);
    else  /* if no thread found - take idle thread for this cpu */
        next_thrd = idle_threads[cpu];

    /* lock discovered thread before switching state */
    thread_lock_thread(next_thrd);

    /* set thread state and put timestamp */
    next_thrd->state = THREAD_STATE_RUNNING;
    next_thrd->sched_stamp = SCHED_TICKS2MSEC(sched_ticks);

    /* switch to next thread */
    arch_sched_context_switch(curr_thrd, next_thrd);
    /* NOTE: For new threads control never goes here after switch at the first
     * time. But thread stub routine calls sched_complete_context_switch()
     * routine directly, so context switch completes inside this routine.
     */

    /* perform last steps of context switch */
    sched_complete_context_switch();
}
