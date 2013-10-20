/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/errors.h>
#include <app/syslib.h>
#include "tests.h"


/***** Virtual memory allocation test *****************************************/

int mem_test_virtmem_alloc(void)
{
/*    unsigned n = 1 * 1024 * 1024; */
    unsigned n = 16 * 1024;
    unsigned i;
    char ptrn = 0x5A;
    char *mem;
    status_t err;

    /* allocate memory block */
    mem = sys_virtmem_alloc(n);
    if(!mem)
        return 0;

    /* fill with pattern, this also generates page faults */
    for(i = 0; i < n; ++i)
        mem[i] = ptrn;

    /* check pattern */
    for(i = 0; i < n; ++i) {
        if(mem[i] != ptrn) {
            sys_virtmem_free(mem);
            return 0;
        }
    }

    /* free virtual memory region */
    err = sys_virtmem_free(mem);

    return err == NO_ERROR ? 1 : 0;
}


/***** Virtual memory allocation stress test **********************************/

#define N_VMEM_BLOCKS 1024
static char* virtmem_sress_alloc_blocks[N_VMEM_BLOCKS];
static unsigned virtmem_stress_alloc_idx = 0;
static unsigned virtmem_stress_free_idx = 0;
static sem_id virtmem_stress_sem;
static int virtmem_stress_exit = 0;
static int virtmem_stress_error = 0;

static int mem_test_virtmem_stress_alloc_thread(void *data)
{
    unsigned n = 16 * 1024;
    unsigned i;
    status_t err;

    while(!virtmem_stress_exit && !virtmem_stress_error) {
        /* acquire semaphore before accessing data */
        err = semaphore_down(virtmem_stress_sem, 1);
        if(err != NO_ERROR) {
            virtmem_stress_error = 1;
            return 0;
        }

        /* check if we reached tail */
        if(virtmem_sress_alloc_blocks[virtmem_stress_alloc_idx] != NULL) {
            semaphore_up(virtmem_stress_sem, 1);
            continue;
        }

        /* allocate memory block */
        virtmem_sress_alloc_blocks[virtmem_stress_alloc_idx] = sys_virtmem_alloc(n);
        if(!virtmem_sress_alloc_blocks[virtmem_stress_alloc_idx]) {
            virtmem_stress_error = 1;
            semaphore_up(virtmem_stress_sem, 1);
            return 0;
        }

        /* generates page faults */
        for(i = 0; i < n; ++i)
            virtmem_sress_alloc_blocks[virtmem_stress_alloc_idx][i] = 0;

        /* update index */
        virtmem_stress_alloc_idx = (virtmem_stress_alloc_idx + 1) % N_VMEM_BLOCKS;

        /* release semaphore */
        semaphore_up(virtmem_stress_sem, 1);
    }

    return 0;
}

static int mem_test_virtmem_stress_free_thread(void *data)
{
    status_t err;

    while(!virtmem_stress_exit && !virtmem_stress_error) {
        /* acquire semaphore before accessing data */
        err = semaphore_down(virtmem_stress_sem, 1);
        if(err != NO_ERROR) {
            virtmem_stress_error = 1;
            return 0;
        }

        /* check if we reached tail */
        if(virtmem_sress_alloc_blocks[virtmem_stress_free_idx] == NULL) {
            semaphore_up(virtmem_stress_sem, 1);
            continue;
        }

        /* free memory block */
        err = sys_virtmem_free(virtmem_sress_alloc_blocks[virtmem_stress_free_idx]);
        if(err != NO_ERROR) {
            virtmem_stress_error = 1;
            semaphore_up(virtmem_stress_sem, 1);
            return 0;
        }
        virtmem_sress_alloc_blocks[virtmem_stress_free_idx] = NULL;

        /* update index */
        virtmem_stress_free_idx = (virtmem_stress_free_idx + 1) % N_VMEM_BLOCKS;

        /* release semaphore */
        semaphore_up(virtmem_stress_sem, 1);
    }

    return 0;
}

int mem_test_virtmem_alloc_stress(void)
{
    thread_id tid;

    /* create semaphore for data structures access */
    virtmem_stress_sem = semaphore_create(NULL, 1, 1);
    if(virtmem_stress_sem == INVALID_SEMID)
        return 0;

    /* create allocator thread */
    tid = sys_create_thread(mem_test_virtmem_stress_alloc_thread, NULL, false, 0);
    if(tid == INVALID_THREADID)
        return 0;

    sys_thread_sleep(500);

    /* create garbage collector thread */
    tid = sys_create_thread(mem_test_virtmem_stress_free_thread, NULL, false, 0);
    if(tid == INVALID_THREADID)
        return 0;

    /* wait while test is executing */
    sys_thread_sleep(5000);

    /* stop test */
    virtmem_stress_exit = 1;
    sys_thread_sleep(500);

    semaphore_delete(virtmem_stress_sem);

    return !virtmem_stress_error;
}
#undef N_VMEM_BLOCKS
