/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/

#include <phlox/syscall.h>

#define GLOBAL(x) .global x; x


/*
 * A table of stubs used by user space code run time. The table is page size
 * and mapped directly to user space.
 *
*/

.text

/* Table start */
GLOBAL(user_stub_table_start):


/* return point from thread function */
GLOBAL(user_stub_thread_exit):
    movl %eax, %ebx                    /* thread function return value */
    movl $SYSCALL_THREAD_EXIT, %eax    /* system call number */
    int  $0xC0                         /* do sys call */
    /* return is not expected */

    movl %eax, %cr0                    /* Simulate GP fault */


/* Table end */
GLOBAL(user_stub_table_end):
