/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/errors.h>
#include <app/syslib.h>
#include "tests.h"


/***** Virtual memory allocation test *****************************************/

int test5(void)
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
