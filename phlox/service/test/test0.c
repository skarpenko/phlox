/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/

#include <phlox/errors.h>
#include <app/syslib.h>
#include "tests.h"


/***** Thread creation test ***************************************************/

static int thread_func(void *data)
{
    int *p = (int*)data;
    *p = 1;
    return 0;
}

int test0(void)
{
    thread_id tid;
    int value = 0;
    int retry = 1000;

    /* create thread */
    tid = sys_create_thread(thread_func, &value, false, 0);
    if(tid == INVALID_THREADID)
        return 0;

    /* wait for thread change the value */
    while(retry-- && !value)
        sys_thread_yield();

    return value ? 1 : 0;
}
