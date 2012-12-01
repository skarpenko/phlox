/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <sys/debug.h>
#include <phlox/errors.h>
#include <phlox/heap.h>
#include <phlox/avl_tree.h>
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

/* Processes tree for fast search by id */
static avl_tree_t processes_tree;

/* Spinlock for operations on processes list / tree */
static spinlock_t processes_lock;

/* Kernel process */
static process_t *kernel_process = NULL;


/*** Locally used routines ***/

/* compare routine for processes AVL tree */
static int compare_process_id(const void *p1, const void *p2)
{
    if( ((process_t *)p1)->id > ((process_t *)p2)->id )
        return 1;
    if( ((process_t *)p1)->id < ((process_t *)p2)->id )
        return -1;
    return 0;
}

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

/* process module init */
status_t process_init(kernel_args_t *kargs)
{
    status_t err;

    /* call arch-specific init */
    err = arch_process_init(kargs);
    if(err != NO_ERROR)
        return err;

    /* valid process ids and processes group ids starts from 1 */
    next_process_id = 1;

    /* data structures spinlock init */
    spin_init(&processes_lock);

    /* list init */
    xlist_init(&processes_list);

    /* tree init */
    avl_tree_create( &processes_tree, compare_process_id,
                     sizeof(process_t),
                     offsetof(process_t, procs_tree_node) );

    return NO_ERROR;
}

/* process module per CPU init */
status_t process_init_per_cpu(kernel_args_t *kargs, uint curr_cpu)
{
    status_t err;

    /* call arch-specific per cpu init */
    err = arch_process_init_per_cpu(kargs, curr_cpu);
    if(err != NO_ERROR)
        return err;

    return NO_ERROR;
}

/* returns kernel process id */
proc_id proc_get_kernel_proc_id(void)
{
    ASSERT_MSG(kernel_process, "Kernel process is not created!");

    return kernel_process->id;
}

/* returns kernel process structure */
process_t *proc_get_kernel_proc(void)
{
    ASSERT_MSG(kernel_process, "Kernel process is not created!");

    /* atomically increment reference count */
    atomic_inc(&kernel_process->ref_count);
    /* return to caller */
    return kernel_process;
}

/* returns process structure by its id */
process_t *proc_get_proc_by_id(proc_id pid)
{
    process_t temp_proc, *proc;
    unsigned int irqs_state;

    temp_proc.id = pid;

    /* acquire lock before tree-search */
    irqs_state = spin_lock_irqsave(&processes_lock);

    /* search */
    proc = avl_tree_find(&processes_tree, &temp_proc, NULL);

    /* increment refs count on success search */
    if(proc) atomic_inc(&proc->ref_count);

    /* release lock */
    spin_unlock_irqrstor(&processes_lock, irqs_state);

    return proc;
}

/* put previously taken process structure */
void proc_put_proc(process_t *proc)
{
    /* decrement references count */
    atomic_dec(&proc->ref_count);

    /* TODO: implement additional functionality */
}
