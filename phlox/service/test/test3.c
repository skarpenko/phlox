/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/

#include <phlox/errors.h>
#include <app/syslib.h>
#include "tests.h"


/***** Threads creation stress test *******************************************/

struct thread_data {
    sem_id sem;
    int counter;
};

static int thread_func(void *data)
{
    status_t err;
    struct thread_data *test_data = (struct thread_data *)data;

    /* acquire semaphore before changing counter */
    err = semaphore_down(test_data->sem, 1);
    if(err != NO_ERROR)
        return 0;

    ++test_data->counter;

    /* release semaphore */
    semaphore_up(test_data->sem, 1);

    return 0;
}

static struct thread_data test_data = {0};

int test3(void)
{
    const int N = 200;
    int retry = 3;
    int i;
    thread_id tid;


    /* create semaphore for counter access */
    test_data.sem = semaphore_create(NULL, 1, 1);
    if(test_data.sem == INVALID_SEMID)
        return 0;

    /* create N threads */
    for(i=0; i < N; ++i) {
        tid = sys_create_thread(thread_func, &test_data, false, 0);
        if(tid == INVALID_THREADID)
            goto test_exit;
    }

    /* wait for counter be equal to number of created threads */
    do {
        sys_thread_sleep(1000);
        if(test_data.counter == N)
            break;
    } while(--retry);

test_exit:
    sys_thread_sleep(1000);

    if(semaphore_delete(test_data.sem) != NO_ERROR)
        return 0;

    return test_data.counter == N ? 1 : 0;
}
