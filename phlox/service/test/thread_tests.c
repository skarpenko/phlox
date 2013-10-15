/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/

#include <phlox/errors.h>
#include <app/syslib.h>
#include "tests.h"


/***** Thread creation test ***************************************************/

static int thread_test_creation_func(void *data)
{
    int *p = (int*)data;
    *p = 1;
    sys_thread_exit(0);
    return 0;
}

int thread_test_creation(void)
{
    thread_id tid;
    int value = 0;
    int retry = 1000;

    /* create thread */
    tid = sys_create_thread(thread_test_creation_func, &value, false, 0);
    if(tid == INVALID_THREADID)
        return 0;

    /* wait for thread change the value */
    while(retry-- && !value)
        sys_thread_yield();

    return value ? 1 : 0;
}


/***** Threads semaphore synchronization **************************************/

struct thread_test_sem_sync_data {
    sem_id sem;
    int *counter;
};

static int thread_test_sem_sync_func0(void *data)
{
    struct thread_test_sem_sync_data *test_data =
        (struct thread_test_sem_sync_data *)data;

    semaphore_down(test_data->sem, 1);

    sys_thread_sleep(10);

    ++(*test_data->counter);

    sys_thread_exit(0);
    return 0;
}

static int thread_test_sem_sync_func1(void *data)
{
    struct thread_test_sem_sync_data *test_data =
        (struct thread_test_sem_sync_data *)data;

    semaphore_down(test_data->sem, 1);

    sys_thread_sleep(20);

    ++(*test_data->counter);

    sys_thread_exit(0);
    return 0;
}

static int thread_test_sem_sync_func2(void *data)
{
    struct thread_test_sem_sync_data *test_data =
        (struct thread_test_sem_sync_data *)data;

    semaphore_down(test_data->sem, 1);

    sys_thread_sleep(30);

    ++(*test_data->counter);

    sys_thread_exit(0);
    return 0;
}

int thread_test_sem_sync(void)
{
    int retry = 1000;
    int counter = 0;
    struct thread_test_sem_sync_data test_data;
    thread_id tid;
    status_t err;

    test_data.counter = &counter;

    /* create semaphore for synchronization */
    test_data.sem = semaphore_create(NULL, 3, 0);
    if(test_data.sem == INVALID_SEMID)
        return 0;

    /* create thread 0 */
    tid = sys_create_thread(thread_test_sem_sync_func0, &test_data, false, 0);
    if(tid == INVALID_THREADID)
        return 0;

    /* create thread 1 */
    tid = sys_create_thread(thread_test_sem_sync_func1, &test_data, false, 0);
    if(tid == INVALID_THREADID)
        return 0;

    /* create thread 2 */
    tid = sys_create_thread(thread_test_sem_sync_func2, &test_data, false, 0);
    if(tid == INVALID_THREADID)
        return 0;

    /* ensure threads reached semaphore */
    sys_thread_sleep(100);

    /* counter should be 0 at this point */
    if(counter)
        return 0;

    /* signal semaphore and wake up waiting threads */
    semaphore_up(test_data.sem, 3);

    /* wait for threads will increment the value */
    while(retry-- && counter != 3)
        sys_thread_yield();

    err = semaphore_delete(test_data.sem);
    if(err != NO_ERROR)
        return 0;

    return counter == 3 ? 1 : 0;
}
