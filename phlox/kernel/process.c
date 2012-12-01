/*
* Copyright 2007-2011, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <sys/debug.h>
#include <phlox/errors.h>
#include <phlox/param.h>
#include <phlox/heap.h>
#include <phlox/avl_tree.h>
#include <phlox/list.h>
#include <phlox/atomic.h>
#include <phlox/spinlock.h>
#include <phlox/vm.h>
#include <phlox/thread.h>
#include <phlox/process.h>
#include <phlox/thread_private.h>


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

/* Process roles properties */
static struct {
    sched_policy_t  def_sched_policy;  /* Default scheduling policy */
    uint            def_prio;          /* Default priority */
} process_roles_props[PROCESS_ROLES_COUNT] = {
  /* Kernel process role properties */
  {
     /* Default scheduling policy */
     { .policy.type = SCHED_POLICY_KERNEL },
     /* Default priority */
     (THREAD_PRIORITY_NORMAL + PROCESS_ROLE_PRIORITY_SHIFT_KERNEL)
  },
  /* Service process role properties */
  {
     /* Default scheduling policy */
     { .policy.type = SCHED_POLICY_SERVICE },
     /* Default priority */
     (THREAD_PRIORITY_NORMAL + PROCESS_ROLE_PRIORITY_SHIFT_SERVICE)
  },
  /* User process role properties */
  {
     /* Default scheduling policy */
     { .policy.type = SCHED_POLICY_ORDINARY },
     /* Default priority */
     (THREAD_PRIORITY_NORMAL + PROCESS_ROLE_PRIORITY_SHIFT_USER)
  }
}; /* Process roles properties */


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

/* put process to the end of processes list */
static void put_process_to_list(process_t *process)
{
    unsigned long irqs_state;

    /* acquire lock before touching bookkeeping structures */
    irqs_state = spin_lock_irqsave(&processes_lock);

    /* add item */
    xlist_add_last(&processes_list, &process->procs_list_node);

    /* put process into avl tree */
    if(!avl_tree_add(&processes_tree, process))
      panic("put_process_to_list(): failed to add process into tree!\n");

    /* release lock */
    spin_unlock_irqrstor(&processes_lock, irqs_state);
}

/* remove process from list */
static void remove_process_from_list(process_t *process)
{
    unsigned long irqs_state;

    /* acquire lock before access */
    irqs_state = spin_lock_irqsave(&processes_lock);

    /* remove process */
    xlist_remove_unsafe(&processes_list, &process->procs_list_node);

    /* remove process from tree */
    if(!avl_tree_remove(&processes_tree, process))
      panic("remove_process_from_list(): failed to remove process from tree!\n");

    /* release lock */
    spin_unlock_irqrstor(&processes_lock, irqs_state);
}

/* common routine for creating processes */
static process_t *create_process_common(const char *name, const char *args)
{
    status_t err;
    process_t *proc;

    /* allocate process structure */
    proc = (process_t *)kmalloc(sizeof(process_t));
    if(!proc)
        return NULL;

    /* init allocated memory with zeroes */
    memset(proc, 0, sizeof(process_t));

    /* init arch-dependend part */
    err = arch_init_process_struct(&proc->arch);
    if(err != NO_ERROR)
        goto error;

    /* if process has name - copy it into structure field */
    if(name) {
        proc->name = kstrdup(name);
        if(!proc->name)
            goto error;
    }

    /* if arguments passed to process - make a copy */
    if(args) {
        proc->args = kstrdup(args);
        if(!proc->args)
            goto error;
    }

    /* init other fields */
    spin_init(&proc->lock);
    proc->state = PROCESS_STATE_BIRTH;

    /* init lists */
    xlist_init(&proc->threads);
    xlist_init(&proc->children);

    /* assign process id */
    proc->id = get_next_process_id();
    proc->gid = proc->id; /* process is a group leader by default */

    /* return result to caller */
    return proc;

error:
    /* release memory on error */
    if(proc->name) kfree(proc->name);
    if(proc->args) kfree(proc->args);
    kfree(proc);

    return NULL; /* failed to create process structure */
}


/*** Public routines ***/

/* process module init */
status_t process_init(kernel_args_t *kargs)
{
    status_t err;
    proc_id pid;

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

    /* create kernel process */
    pid = proc_create_kernel_process("kernel_process");
    if(pid == INVALID_PROCESSID)
        return ERR_MT_GENERAL;

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

/* creates kernel process */
proc_id proc_create_kernel_process(const char* name)
{
    process_t *proc;
    proc_id id;

    ASSERT_MSG(name != NULL && strlen(name) <= SYS_MAX_OS_NAME_LEN,
        "proc_create_kernel_process: kernel process name is invalid!\n");

    /* ensure that kernel process was not created before */
    if(kernel_process)
        panic("proc_create_kernel_process: kernel process already exists!\n");

    /* create kernel process struct */
    proc = create_process_common(name, NULL);
    if(!proc)
        return INVALID_PROCESSID;

    /* final fields init */
    proc->state = PROCESS_STATE_NORMAL;
    id = proc->id;
    proc->aid = vm_get_kernel_aspace_id();
    proc->aspace = vm_get_kernel_aspace();
    proc->process_role = PROCESS_ROLE_KERNEL;
    proc->def_prio = process_roles_props[proc->process_role].def_prio;
    proc->def_sched_policy.raw =
        process_roles_props[proc->process_role].def_sched_policy.raw;

    /* store as global */
    kernel_process = proc;

    /* add to processes list */
    put_process_to_list(proc);

    return id; /* return id to caller */
}

/* attaches new thread to process */
void proc_attach_thread(process_t *proc, thread_t *thread)
{
    unsigned long irqs_state;

    ASSERT_MSG(thread->lock != 0,
        "proc_attach_thread(): thread was not locked before!");

    /* acquire lock before working with process data */
    irqs_state = spin_lock_irqsave(&proc->lock);

    /* add thread to list */
    xlist_add_last(&proc->threads, &thread->proc_list_node);

    /* release lock */
    spin_unlock_irqrstor(&proc->lock, irqs_state);
}

/* detaches thread from process */
void proc_detach_thread(process_t *proc, thread_t *thread)
{
    unsigned long irqs_state;

    ASSERT_MSG(thread->lock != 0,
        "proc_detach_thread(): thread was not locked before!");

    /* acquire lock before touching process data */
    irqs_state = spin_lock_irqsave(&proc->lock);

    /* remove thread from list */
    xlist_remove(&proc->threads, &thread->proc_list_node);

    /* release lock */
    spin_unlock_irqrstor(&proc->lock, irqs_state);
}

/* returns kernel process id */
proc_id proc_get_kernel_process_id(void)
{
    ASSERT_MSG(kernel_process, "Kernel process is not created!");

    return kernel_process->id;
}

/* returns kernel process structure */
process_t *proc_get_kernel_process(void)
{
    ASSERT_MSG(kernel_process, "Kernel process is not created!");

    /* atomically increment reference count */
    atomic_inc(&kernel_process->ref_count);
    /* return to caller */
    return kernel_process;
}

/* returns process structure by its id */
process_t *proc_get_process_by_id(proc_id pid)
{
    process_t *look4, *proc;
    unsigned int irqs_state;

    /* look for */
    look4 = containerof(&pid, process_t, id);

    /* acquire lock before tree-search */
    irqs_state = spin_lock_irqsave(&processes_lock);

    /* search */
    proc = avl_tree_find(&processes_tree, look4, NULL);

    /* increment refs count on success search */
    if(proc) atomic_inc(&proc->ref_count);

    /* release lock */
    spin_unlock_irqrstor(&processes_lock, irqs_state);

    return proc;
}

/* return current process id */
proc_id proc_get_current_process_id(void)
{
    return thread_get_current_thread()->process->id;
}

/* return current process structure */
process_t *proc_get_current_process(void)
{
    process_t *proc = thread_get_current_thread()->process;

    /* atomically increment reference count */
    atomic_inc(&proc->ref_count);

    /* return to caller */
    return proc;
}

/* put previously taken process structure */
void proc_put_process(process_t *proc)
{
    /* decrement references count */
    atomic_dec(&proc->ref_count);

    /* TODO: implement additional functionality */
}
