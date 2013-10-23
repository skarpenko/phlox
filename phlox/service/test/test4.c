/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/errors.h>
#include <app/syslib.h>
#include "tests.h"


/***** Semaphore signaling test ***********************************************/

static int thread_func(void *data)
{
    sem_id *psid = (sem_id*)data;

    /* signal semaphore */
    semaphore_up(*psid, 1);

    return 0;
}

int test4(void)
{
    thread_id tid;
    sem_id sid;
    status_t err;
    int res = 1;

    /* create new semaphore */
    sid = semaphore_create(NULL, 1, 0);
    if(sid == INVALID_SEMID)
        return 0;

    /* create thread */
    tid = sys_create_thread(thread_func, &sid, false, 0);
    if(tid == INVALID_THREADID)
        return 0;

    /* wait for created thread count up semaphore */
    err = semaphore_down_timeout(sid, 1, 1000);
    if(err != NO_ERROR)
        res = 0;

    /* delete semaphore */
    err = semaphore_delete(sid);
    if(err != NO_ERROR)
        res = 0;

    return res;
}
