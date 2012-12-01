#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sched.h"
#include "include/thread_types.h"
#include "include/thread_private.h"
#include "thread.h"

#define NPROC 10
#define NTHREAD 10000

process_t processes[NPROC];
thread_t threads[NTHREAD];
thread_t *pthreads[NTHREAD];

uint next_free_thread=0;
uint free_id=1;

uint randn(uint max)
{
    return (uint)((double)rand() /RAND_MAX * max);
}

void thread_init()
{
    int i;
    memset(processes, 0, sizeof(processes));
    memset(threads, 0, sizeof(threads));

    for(i=0; i<NTHREAD; ++i)
        pthreads[i] = &threads[i];

    srand(time(0));
}

void create_thread(uint prio, uint policy, uint proc)
{
    thread_t *th = &threads[next_free_thread++];
    th->id = free_id++;
    th->s_prio = prio;
    th->sched_policy.raw = policy;
    th->process = &processes[proc];

    sched_add_new_thread(th);
}

void create_threads(uint num)
{
    uint i;
    uint policy = SCHED_POLICY_ORDINARY;
    uint prio;
    uint proc;
//    num--;

    for(i=0; i<num; ++i) {
        prio = randn(THREAD_NUM_PRIORITY_LEVELS);
        proc = randn(NPROC);
        create_thread(prio, policy, proc);
    }

//   create_thread(120/*prio*/, SCHED_POLICY_INTERRUPT, 0);
//   create_thread(50/*prio*/, SCHED_POLICY_KERNEL, 0);
//   create_thread(50/*prio*/, SCHED_POLICY_SERVICE, 0);
//   create_thread(50/*prio*/, SCHED_POLICY_ORDINARY, 0);
}

int thread_sprio_cmp(const void *t1, const void *t2)
{
    thread_t **th1 = (thread_t **)t1;
    thread_t **th2 = (thread_t **)t2;
    
    if((**th1).s_prio < (**th2).s_prio)
        return 1;
    else if((**th1).s_prio > (**th2).s_prio)
        return -1;
    else
        return 0;
    
}

void sort_threads()
{
    qsort(pthreads, next_free_thread, sizeof(thread_t *), thread_sprio_cmp);
}

void dump_threads()
{
    int i;
    sort_threads();

    for(i=0; i < next_free_thread; ++i) {
        printf("id=%4d(%1d)(%d)\ts_pr=%4d\td_pr=%4d\tx_cnt=%4d\tmw=%4d\tmxe=%4d\tmne=%4d\n",
            pthreads[i]->id,
            pthreads[i]->sched_policy.raw,
            pthreads[i]->carrots_sticks,
            pthreads[i]->s_prio,
            pthreads[i]->d_prio,
            pthreads[i]->exe_count,
            (long)pthreads[i]->max_wait,
            (long)pthreads[i]->max_exec,
            (long)pthreads[i]->min_exec
        );
    }
}
