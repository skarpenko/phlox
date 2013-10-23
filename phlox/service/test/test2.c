/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/

#include <phlox/errors.h>
#include <app/syslib.h>
#include "tests.h"


/***** Thread suspend / resume ************************************************/

struct thread_data {
    volatile int counter;
    volatile int exit;
};

static int thread_func(void *data)
{
    struct thread_data *test_data = (struct thread_data *)data;

    ++test_data->counter;
    thread_suspend_current();

    ++test_data->counter;
    thread_suspend_current();

    /* set counter to 0 while not exit condition, main thread should suspend this
     * thread and ensure that it is really suspended by changing counter to 2 and
     * testing if it is still 2.
     */
    while(!test_data->exit)
        test_data->counter = 0;

    return 0;
}

static struct thread_data test_data = {0};

int test2(void)
{
    thread_id tid;
    status_t err;

    /* create thread in suspended state  */
    tid = sys_create_thread(thread_func, &test_data, true, 0);
    if(tid == INVALID_THREADID)
        return 0;

    /* wait before counter check */
    sys_thread_sleep(100);

    /* counter should be 0 at this point */
    if(test_data.counter != 0)
        return 0;

    /* resume thread */
    err = sys_thread_resume(tid);
    if(err != NO_ERROR)
        return 0;

    /* wait before counter check */
    sys_thread_sleep(100);

    /* counter should be 1 at this point */
    if(test_data.counter != 1)
        return 0;

    /* resume thread again */
    err = sys_thread_resume(tid);
    if(err != NO_ERROR)
        return 0;

    /* wait before counter check */
    sys_thread_sleep(100);

    /* counter should be 2 at this point */
    if(test_data.counter != 2)
        return 0;

    /* resume thread again */
    err = sys_thread_resume(tid);
    if(err != NO_ERROR)
        return 0;

    /* wait before counter check */
    sys_thread_sleep(100);

    /* counter should be 0 at this point */
    if(test_data.counter != 0)
        return 0;

    /* suspend thread */
    err = sys_thread_suspend(tid);
    if(err != NO_ERROR)
        return 0;

    /* wait before changing counter */
    sys_thread_sleep(100);

    test_data.counter = 2;

    /* wait before counter check */
    sys_thread_sleep(100);

    /* counter should be 2 at this point */
    if(test_data.counter != 2)
        return 0;

    /* resume thread again */
    err = sys_thread_resume(tid);
    if(err != NO_ERROR)
        return 0;

    test_data.exit = 1; /* created thread may now exit after resume */

    /* wait before return */
    sys_thread_sleep(100);

    return 1;
}
