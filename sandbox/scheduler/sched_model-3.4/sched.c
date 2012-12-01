// TODO: Enhance xlist.c peek_next / peek_prev

#include <stdio.h>
#include <thread_private.h>
#include "sched.h"
#include "sched_private.h"
#include "include/thread_types.h"

bigtime_t sched_ticks = 0;
thread_t *running = NULL;
runqueue_t runqueues[1];
int quanta = THREAD_DEFAULT_QUANTA;
uint factor = THREAD_DEFAULT_QUANTA / SCHED_FACTOR_BMAX;


static void sched_shuffle_runqueue_head(runqueue_t *rq, int tick_type);
static void sched_put_thread(runqueue_t *rq, thread_t *th);

// inline
static thread_t *sched_peek_head_thread(runqueue_t *rq, int prio)
{
    list_elem_t *tmp = xlist_peek_first(&rq->queue[prio]);
    return containerof(tmp, thread_t, sched_list_node);
}

// inline
static thread_t *sched_peek_next_thread(thread_t *th)
{
    list_elem_t *tmp = th->sched_list_node.next;
    return (tmp!=NULL) ? containerof(tmp, thread_t, sched_list_node) : NULL;
}

// inline
static thread_t *sched_pop_head_thread(runqueue_t *rq, int prio)
{
    list_elem_t *tmp = xlist_extract_first(&rq->queue[prio]);
    thread_t *th = containerof(tmp, thread_t, sched_list_node);
    rq->total_count--;
    return th;
}

// inline
static thread_t *sched_extract_thread(runqueue_t *rq, thread_t *th)
{
    xlist_remove(&rq->queue[th->d_prio], &th->sched_list_node);
    rq->total_count--;
    return th;
}

static thread_t *sched_significant_thread(thread_t *t1, thread_t *t2)
{
    uint p1, p2;

    /*
// #if 0 ???
    if(t1->d_prio > t2->d_prio)
        return t1;
    else if (t2->d_prio > t1->d_prio)
        return t2;
*/
    // policy compare

    if(t1->sched_policy.policy.type == SCHED_POLICY_INTERACTIVE &&
       !(t1->process->flags & PROCESS_FLAG_INTERACTIVE) )
        p1 = SCHED_POLICY_ORDINARY;
    else
        p1 = t1->sched_policy.policy.type;

    if(t2->sched_policy.policy.type == SCHED_POLICY_INTERACTIVE &&
       !(t2->process->flags & PROCESS_FLAG_INTERACTIVE) )
        p2 = SCHED_POLICY_ORDINARY;
    else
        p2 = t2->sched_policy.policy.type;

    if (p1 >= p2)
        return t1;
    else
        return t2;
}

// inline
static int sched_thread_penalty(thread_t *th)
{
    static int sys_bonus[10] = { 0, 0, 0, 0, 0, 0,  0,  0, -1, -1 };
    static int ord_bonus[10] = { 3, 2, 2, 1, 0, 0, -1, -2, -3, -3 };
    const int n = sizeof(sys_bonus)/sizeof(int);
    int *bonus;
    int ptyi;

    if(th->sched_policy.policy.type == SCHED_POLICY_INTERRUPT ||
       th->sched_policy.policy.type == SCHED_POLICY_HW_HANDLER)
        bonus = sys_bonus;
    else
        bonus = ord_bonus;


    ptyi = 100 * (running->ijiffies - th->jiffies) / th->ijiffies / n;
    if(ptyi < 0)       ptyi = 0;
    else if(ptyi >= n) ptyi = n-1;

    if(bonus == ord_bonus) {
        th->carrots_sticks += bonus[ptyi];
        if(!bonus[ptyi])
            if(th->carrots_sticks > 0) th->carrots_sticks--;
        else
            if(th->carrots_sticks < 0) th->carrots_sticks++;

        if (th->carrots_sticks > SCHED_MAX_THREAD_BONUS)
            th->carrots_sticks = SCHED_MAX_THREAD_BONUS;
        else if (th->carrots_sticks < -SCHED_MAX_THREAD_PENALTY)
            th->carrots_sticks = -SCHED_MAX_THREAD_PENALTY;
    } else {
        th->carrots_sticks = bonus[ptyi];
    }
    return th->carrots_sticks;
}

void sched_init_runqueue(runqueue_t * rq)
{
    int i;
    rq->lock = 0;
    rq->total_count = 0;
    for(i=0; i < THREAD_NUM_PRIORITY_LEVELS; i++)
        xlist_init(&rq->queue[i]);
}

void sched_init()
{
    sched_init_runqueue(&runqueues[0]);
}

int lost_ticks = 0;
int in_sched_tick = 0;
int tick_type = 0;

void sched_tick()
{
    int ticks_out;
    int cpu = 0;
    int resched = 0;
    int prio;

    if(in_sched_tick) {
        lost_ticks++;
        return;
    } else
        in_sched_tick = 1;

    sched_ticks += (ticks_out = lost_ticks + 1);
    lost_ticks = 0;

    tick_type = ++tick_type & SCHED_TICK_TYPES_MASK;
    sched_shuffle_runqueue_head(&runqueues[cpu], tick_type);
    if(tick_type == SCHED_ESTIMATION_TICK) {
        uint a, b;
        quanta = THREAD_DEFAULT_QUANTA - (runqueues[cpu].total_count >> SCHED_QUANTA_THRESHOLD2);
        if (quanta < THREAD_MINIMUM_QUANTA) quanta = THREAD_MINIMUM_QUANTA;

        a = runqueues[cpu].total_count / SCHED_FACTOR_A; if(a==0) a = 1;
        b = (!runqueues[cpu].total_count) ? SCHED_FACTOR_BMAX : SCHED_FACTOR_BTHR / runqueues[cpu].total_count;
        if (b > SCHED_FACTOR_BMAX) b = SCHED_FACTOR_BMAX; if(b==0) b = 1;
        factor = a*SCHED_FACTOR_BASE/b;
    }

    if( !running->cpu_captured && !(running->flags & THREAD_FLAG_RESCHEDULE) ) {
        if( ((running->jiffies -= ticks_out) <= 0) ? running->jiffies = 0, 1 : 0 )
            resched = 1;
        else if (SCHED_TICKS2MSEC(running->ijiffies-running->jiffies) >= THREAD_QUANTA_GRANULARITY)
            for(prio=THREAD_NUM_PRIORITY_LEVELS - 1; prio > running->d_prio; --prio)
                if (runqueues[cpu].queue[prio].count) {
                    resched = 1;
                    break;
                }

        if(resched)
            running->flags |= THREAD_FLAG_RESCHEDULE;
    }

    in_sched_tick = 0;


    // this part executes in interrupt entry ?
    if (resched)
        sched_resched();
}

static void sched_put_thread(runqueue_t *rq, thread_t *th)
{
    switch(th->state) {
        case THREAD_STATE_READY:
            xlist_add_last(&rq->queue[th->d_prio], &th->sched_list_node);
            rq->total_count++;
            break;
        case THREAD_STATE_SUSPENDED:
        case THREAD_STATE_WAITING:
        default:
            printf("FATAL ERROR IN sched_add_thread (%d)\n", th->state);
    }
}

static uint max_pr_up = 0;
static uint g_queue_bonus = 0;
void sched_shuffle_runqueue_head(runqueue_t *rq, int tick_type)
{
    thread_t *th;
    int queue_bonus = 0;
    int prio;
    int new_prio;
    bigtime_t ticks_msec = SCHED_TICKS2MSEC(sched_ticks);

    for (prio = tick_type; prio < THREAD_NUM_PRIORITY_LEVELS - 1; prio+=SCHED_TICK_TYPES_COUNT) {
        if(!rq->queue[prio].count) continue;
        th = sched_peek_head_thread(rq, prio);

        queue_bonus = rq->queue[prio].count >> SCHED_QUEUE_BONUS_THRESHOLD2;
        new_prio = th->s_prio + ((ticks_msec - th->sched_stamp) / factor) + queue_bonus + th->carrots_sticks; /* add carrots_sticks */

        if(new_prio > th->d_prio) {
            if(new_prio > THREAD_NUM_PRIORITY_LEVELS - 1)
                new_prio = THREAD_NUM_PRIORITY_LEVELS - 1;

            sched_pop_head_thread(rq, prio);

            if(max_pr_up < (new_prio - th->d_prio)) {
                max_pr_up = new_prio - th->d_prio;
                g_queue_bonus = queue_bonus;
            }

            th->d_prio = new_prio;

            sched_put_thread(rq, th);
        }
    }
}

void sched_resched()
{
    int prio;
    thread_t *th = NULL;
    runqueue_t *rq = &runqueues[0];

    running->state = THREAD_STATE_READY;
    /*stat*/
    if(running->max_exec < SCHED_TICKS2MSEC(sched_ticks) - running->sched_stamp)
        running->max_exec = SCHED_TICKS2MSEC(sched_ticks) - running->sched_stamp;
    if(running->min_exec > SCHED_TICKS2MSEC(sched_ticks) - running->sched_stamp)
        running->min_exec = SCHED_TICKS2MSEC(sched_ticks) - running->sched_stamp;
    /**/
    running->sched_stamp = SCHED_TICKS2MSEC(sched_ticks);
    running->flags &= ~THREAD_FLAG_RESCHEDULE;

   /* PENALTY OR BONUS */
   running->d_prio = running->s_prio + sched_thread_penalty(running);
   if(running->d_prio > THREAD_NUM_PRIORITY_LEVELS-1)
       running->d_prio = THREAD_NUM_PRIORITY_LEVELS-1;
   else if(running->d_prio < THREAD_LOWEST_PIORITY) // min priority constant
       running->d_prio = THREAD_LOWEST_PIORITY;
   /**/

   sched_put_thread(rq, running);
   running = NULL;

    for(prio=THREAD_NUM_PRIORITY_LEVELS-1; prio>=0; prio--) {
        if(rq->queue[prio].count) {
            if(rq->queue[prio].count > 1) {
                thread_t *th_next;
                uint n = SCHED_QUEUE_LOOKAHEAD_DEPTH;
                th = sched_peek_head_thread(rq, prio);
                th_next = th;
                while( (th_next = sched_peek_next_thread(th_next)) != NULL && --n )
                    th = sched_significant_thread(th, th_next);
                sched_extract_thread(rq, th);
            }
            else
                th = sched_pop_head_thread(rq, prio);
            break;
        }
    }

    if (th != NULL) {
        th->jiffies = th->ijiffies = SCHED_MSEC2TICKS(quanta);
        th->exe_count++;
        th->state = THREAD_STATE_RUNNING;

        /*stat*/
        if(th->max_wait < SCHED_TICKS2MSEC(sched_ticks) - th->sched_stamp)
            th->max_wait = SCHED_TICKS2MSEC(sched_ticks) - th->sched_stamp;
        /**/

        th->sched_stamp = SCHED_TICKS2MSEC(sched_ticks);
        running = th;
    } else {
        // select idle
    }
}

void sched_capture_cpu()
{

}

void sched_release_cpu()
{

}

void sched_remove_thread(thread_t *thread)
{
}

void sched_add_new_thread(thread_t *th)
{
    if (th->s_prio > THREAD_NUM_PRIORITY_LEVELS-1)
        th->s_prio = THREAD_NUM_PRIORITY_LEVELS-1;

    if(running == NULL) {
        th->jiffies = th->ijiffies = SCHED_MSEC2TICKS(quanta);
        th->exe_count++;
        th->state = THREAD_STATE_RUNNING;
        th->sched_stamp = SCHED_TICKS2MSEC(sched_ticks); // important step
        th->d_prio = th->s_prio;
        th->min_exec = (1000000);
        running = th;
    } else {
        th->state = THREAD_STATE_READY;
        th->sched_stamp = SCHED_TICKS2MSEC(sched_ticks); // important step
        th->d_prio = th->s_prio;
        th->min_exec = (1000000);
        sched_put_thread(&runqueues[0], th);
    }
}

void dump_queues()
{
    int i;

    for(i=0; i<THREAD_NUM_PRIORITY_LEVELS-1; i++)
        printf("q_pri=%d, count=%d\n",
                i,
                runqueues[0].queue[i].count
              );
}

void dump_specials()
{
    printf("max_up = %d, queue_bonus = %d, quanta=%d, factor=%d\n", max_pr_up, g_queue_bonus, quanta, factor);
}
