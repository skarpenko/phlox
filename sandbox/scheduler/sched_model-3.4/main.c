#include "sched.h"
#include "thread.h"

#define TICKS_COUNT 1000000

int main()
{
    int tick;

    sched_init();
    thread_init();

    create_threads(20);

    for(tick = 0; tick < TICKS_COUNT; ++tick)
        sched_tick();

    dump_threads();
//    dump_queues();
    dump_specials();

    return 0;
}
