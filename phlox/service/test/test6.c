/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/errors.h>
#include <app/syslib.h>
#include "tests.h"


/***** Virtual memory allocation stress test **********************************/

#define N_VMEM_BLOCKS 1024
static char* alloc_blocks[N_VMEM_BLOCKS];
static unsigned alloc_idx = 0;
static unsigned free_idx = 0;
static sem_id sem;
static int test_exit = 0;
static int test_error = 0;

static int producer_thread_func(void *data)
{
    unsigned n = 16 * 1024;
    unsigned i;
    status_t err;

    while(!test_exit && !test_error) {
        /* acquire semaphore before accessing shared data */
        err = semaphore_down(sem, 1);
        if(err != NO_ERROR) {
            test_error = 1;
            return 0;
        }

        /* check if we reached tail */
        if(alloc_blocks[alloc_idx] != NULL) {
            semaphore_up(sem, 1);
            continue;
        }

        /* allocate memory block */
        alloc_blocks[alloc_idx] = sys_virtmem_alloc(n);
        if(!alloc_blocks[alloc_idx]) {
            test_error = 1;
            semaphore_up(sem, 1);
            return 0;
        }

        /* generates page faults */
        for(i = 0; i < n; ++i)
            alloc_blocks[alloc_idx][i] = 0;

        /* update index */
        alloc_idx = (alloc_idx + 1) % N_VMEM_BLOCKS;

        /* release semaphore */
        semaphore_up(sem, 1);
    }

    return 0;
}

static int consumer_thread_func(void *data)
{
    status_t err;

    while(!test_exit && !test_error) {
        /* acquire semaphore before accessing shared data */
        err = semaphore_down(sem, 1);
        if(err != NO_ERROR) {
            test_error = 1;
            return 0;
        }

        /* check if we reached head */
        if(alloc_blocks[free_idx] == NULL) {
            semaphore_up(sem, 1);
            continue;
        }

        /* free memory block */
        err = sys_virtmem_free(alloc_blocks[free_idx]);
        if(err != NO_ERROR) {
            test_error = 1;
            semaphore_up(sem, 1);
            return 0;
        }
        alloc_blocks[free_idx] = NULL;

        /* update index */
        free_idx = (free_idx + 1) % N_VMEM_BLOCKS;

        /* release semaphore */
        semaphore_up(sem, 1);
    }

    return 0;
}

int test6(void)
{
    thread_id tid;

    /* create semaphore for shared data structures access */
    sem = semaphore_create(NULL, 1, 1);
    if(sem == INVALID_SEMID)
        return 0;

    /* create allocator thread */
    tid = sys_create_thread(producer_thread_func, NULL, false, 0);
    if(tid == INVALID_THREADID)
        return 0;

    /* let allocator thread to allocate memory */
    sys_thread_sleep(500);

    /* create garbage collector thread */
    tid = sys_create_thread(consumer_thread_func, NULL, false, 0);
    if(tid == INVALID_THREADID)
        return 0;

    /* wait while test is executing */
    sys_thread_sleep(5000);

    /* stop test */
    test_exit = 1;
    sys_thread_sleep(500);

    if(semaphore_delete(sem) != NO_ERROR)
        return 0;

    return !test_error;
}
