/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/

#include <phlox/errors.h>
#include <app/syslib.h>
#include "tests.h"


/***** Threads semaphore synchronization **************************************/

struct thread_data {
    sem_id sem;
    int counter;
};

static int thread_func(void *data)
{
    struct thread_data *test_data = (struct thread_data *)data;
    unsigned a = (unsigned)sys_current_thread_id() & 0xF;

    semaphore_down(test_data->sem, 1);

    sys_thread_sleep(a*10);

    ++test_data->counter;

    return 0;
}

static struct thread_data test_data = { 0, 0 };

int test1(void)
{
    int retry = 100;
    thread_id tid;
    status_t err;

    /* create semaphore for synchronization */
    test_data.sem = semaphore_create(NULL, 5, 0);
    if(test_data.sem == INVALID_SEMID)
        return 0;

    /* create thread 0 */
    tid = sys_create_thread(thread_func, &test_data, false, 0);
    if(tid == INVALID_THREADID)
        return 0;

    /* create thread 1 */
    tid = sys_create_thread(thread_func, &test_data, false, 0);
    if(tid == INVALID_THREADID)
        return 0;

    /* create thread 2 */
    tid = sys_create_thread(thread_func, &test_data, false, 0);
    if(tid == INVALID_THREADID)
        return 0;

    /* create thread 3 */
    tid = sys_create_thread(thread_func, &test_data, false, 0);
    if(tid == INVALID_THREADID)
        return 0;

    /* create thread 4 */
    tid = sys_create_thread(thread_func, &test_data, false, 0);
    if(tid == INVALID_THREADID)
        return 0;

    /* ensure threads reached semaphore */
    sys_thread_sleep(1000);

    /* counter should be 0 at this point */
    if(test_data.counter)
        return 0;

    /* signal semaphore and wake up waiting threads */
    semaphore_up(test_data.sem, 5);

    /* wait for threads will increment the value */
    while(retry-- && test_data.counter != 5)
        sys_thread_sleep(5);

    err = semaphore_delete(test_data.sem);
    if(err != NO_ERROR)
        return 0;

    return test_data.counter == 5 ? 1 : 0;
}
