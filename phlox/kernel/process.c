/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <sys/debug.h>
#include <phlox/errors.h>
#include <phlox/heap.h>
#include <phlox/list.h>
#include <phlox/atomic.h>
#include <phlox/spinlock.h>
#include <phlox/thread_private.h>
#include <phlox/process.h>


/* Redefinition for convenience */
typedef xlist_t processes_list_t;

/* Next available process id */
static vuint next_process_id;

/* Processes list */
static processes_list_t processes_list;

/* Spinlock for operations on processes list */
static spinlock_t processes_lock;

/* Kernel process */
static process_t *kernel_process = NULL;


/*** Locally used routines ***/

/* returns available process id */
static proc_id get_next_process_id(void)
{
    proc_id retval;

    /* atomically increment and get previous value */
    retval = (proc_id)atomic_inc_ret(&next_process_id);
    if(retval == INVALID_PROCESSID)
        panic("No available process IDs!");
    /* TODO: Implement better approach for reliability. */

    return retval;
}


/*** Public routines ***/

/* returns kernel process id */
proc_id proc_get_kernel_proc_id(void)
{
    return kernel_process->id;
}

/* returns kernel process structure */
process_t *proc_get_kernel_proc(void)
{
    return kernel_process;
}
