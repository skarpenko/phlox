#ifndef __SCHED_H__
#define __SCHED_H__

#include <thread_types.h>

void sched_init();
void sched_tick();
void sched_resched();
void sched_add_new_thread(thread_t *th);
void dump_queues();
void dump_specials();

#endif
