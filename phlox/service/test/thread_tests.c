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

    return 0;
}

static int thread_test_sem_sync_func1(void *data)
{
    struct thread_test_sem_sync_data *test_data =
        (struct thread_test_sem_sync_data *)data;

    semaphore_down(test_data->sem, 1);

    sys_thread_sleep(20);

    ++(*test_data->counter);

    return 0;
}

static int thread_test_sem_sync_func2(void *data)
{
    struct thread_test_sem_sync_data *test_data =
        (struct thread_test_sem_sync_data *)data;

    semaphore_down(test_data->sem, 1);

    sys_thread_sleep(30);

    ++(*test_data->counter);

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


/***** Thread suspend / resume ************************************************/


static int thread_test_suspend_resume_func0(void *data)
{
    volatile int *ctr = (volatile int*)data;

    ++*ctr;
    thread_suspend_current();

    ++*ctr;
    thread_suspend_current();

    /* set counter to 0 while it's not 3, main thread should suspend this thread
     * and ensure that is really suspended by changing counter to 2 and testing
     * if it is still 2.
     */
    while(*ctr != 3)
        *ctr = 0;

    return 0;
}

int thread_test_suspend_resume(void)
{
    int counter = 0;
    thread_id tid;
    status_t err;

    /* create thread in suspended state  */
    tid = sys_create_thread(thread_test_suspend_resume_func0, &counter,
        true, 0);
    if(tid == INVALID_THREADID)
        return 0;

    /* wait before counter check */
    sys_thread_sleep(100);

    /* counter should be 0 at this point */
    if(counter != 0)
        return 0;

    /* resume thread */
    err = sys_thread_resume(tid);
    if(err != NO_ERROR)
        return 0;

    /* wait before counter check */
    sys_thread_sleep(100);

    /* counter should be 1 at this point */
    if(counter != 1)
        return 0;

    /* resume thread again */
    err = sys_thread_resume(tid);
    if(err != NO_ERROR)
        return 0;

    /* wait before counter check */
    sys_thread_sleep(100);

    /* counter should be 2 at this point */
    if(counter != 2)
        return 0;

    /* resume thread again */
    err = sys_thread_resume(tid);
    if(err != NO_ERROR)
        return 0;

    /* wait before counter check */
    sys_thread_sleep(100);

    /* counter should be 0 at this point */
    if(counter != 0)
        return 0;

    /* suspend thread */
    err = sys_thread_suspend(tid);
    if(err != NO_ERROR)
        return 0;

    /* wait before changing counter */
    sys_thread_sleep(100);
    counter = 2;
    /* wait before counter check */
    sys_thread_sleep(100);

    /* counter should be 0 at this point */
    if(counter != 2)
        return 0;

    counter = 3; /* created thread may now exit after resume */

    /* resume thread again */
    err = sys_thread_resume(tid);
    if(err != NO_ERROR)
        return 0;

    return 1;
}


/***** Threads creation stress test *******************************************/


struct thread_test_creat_stress_data {
    sem_id sem;
    int *counter;
};

static int thread_test_creation_stress_func0(void *data)
{
    status_t err;
    struct thread_test_creat_stress_data *stress_data =
        (struct thread_test_creat_stress_data *)data;

    /* acquire semaphore before changing counter */
    err = semaphore_down(stress_data->sem, 1);
    if(err != NO_ERROR)
        return 0;

    ++*stress_data->counter;

    /* release semaphore */
    semaphore_up(stress_data->sem, 1);

    return 0;
}

int thread_test_creation_stress(void)
{
    const int N = 100;
    int retry = 3;
    int i;
    int counter = 0;
    struct thread_test_creat_stress_data stress_data;
    thread_id tid;

    stress_data.counter = &counter;

    /* create semaphore for counter access */
    stress_data.sem = semaphore_create(NULL, 1, 1);
    if(stress_data.sem == INVALID_SEMID)
        return 0;

    /* create N threads */
    for(i=0; i < N; ++i) {
        tid = sys_create_thread(thread_test_creation_stress_func0, &stress_data,
            false, 0);
        if(tid == INVALID_THREADID)
            goto test_exit;
    }

    /* wait for counter be equal to number of created threads */
    do {
        sys_thread_sleep(1000);
        if(counter == N)
            break;
    } while(--retry);

test_exit:
    sys_thread_sleep(1000);

    semaphore_delete(stress_data.sem);

    return counter == N ? 1 : 0;
}
