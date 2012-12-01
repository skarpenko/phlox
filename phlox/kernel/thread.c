/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <sys/debug.h>
#include <phlox/kernel.h>
#include <phlox/param.h>
#include <phlox/errors.h>
#include <phlox/heap.h>
#include <phlox/list.h>
#include <phlox/avl_tree.h>
#include <phlox/atomic.h>
#include <phlox/spinlock.h>
#include <phlox/processor.h>
#include <phlox/vm.h>
#include <phlox/scheduler.h>
#include <phlox/process.h>
#include <phlox/thread.h>
#include <phlox/thread_private.h>


/* Redefinition for convenience */
typedef xlist_t threads_list_t;

/* Thread lists types */
enum list_types {
    ALIVE_THREADS_LIST,  /* List of alive threads */
    DEAD_THREADS_LIST    /* List of dead threads */
};

/* Next available thread id */
static vuint next_thread_id;

/* Threads list */
static threads_list_t threads_list;

/* Tree of threads for fast search by id */
static avl_tree_t threads_tree;

/* Dead threads list (for faster threads creation) */
static threads_list_t dead_threads_list;

/* Spinlock for operations on treads lists and tree */
static spinlock_t threads_lock;


/*** Locally used routines ***/

/* compare routine for threads AVL tree */
static int compare_thread_id(const void *t1, const void *t2)
{
    if( ((thread_t *)t1)->id > ((thread_t *)t2)->id )
        return 1;
    if( ((thread_t *)t1)->id < ((thread_t *)t2)->id )
        return -1;
    return 0;
}

/* returns available thread id */
static thread_id get_next_thread_id(void)
{
    thread_id retval;

    /* atomically increment and get previous value */
    retval = (thread_id)atomic_inc_ret(&next_thread_id);
    if(retval == INVALID_THREADID)
        panic("No available thread IDs!");
    /* TODO: Implement better approach for reliability. */

    return retval;
}

/* put new thread into threads list (no lock acquired) */
static void put_thread_to_list_nolock(thread_t *thread)
{
    /* thread must be in Birth state */
    ASSERT_MSG(thread->state == THREAD_STATE_BIRTH,
        "put_thread_to_list_nolock(): Wrong thread state!");

    /* add thread to list */
    xlist_add_last(&threads_list, &thread->threads_list_node);

    /* put thread into avl tree */
    if(!avl_tree_add(&threads_tree, thread))
      panic("put_thread_to_list_nolock(): failed to add thread into tree!\n");
}

/* put new thread into threads list */
static void put_thread_to_list(thread_t *thread)
{
    unsigned long irqs_state;

    /* acquire lock before call to non-lock version */
    irqs_state = spin_lock_irqsave(&threads_lock);

    /* call non-lock version */
    put_thread_to_list_nolock(thread);

    /* release lock */
    spin_unlock_irqrstor(&threads_lock, irqs_state);
}

/* move thread from one list to another (non-lock version) */
static void move_thread_to_list_nolock(thread_t *thread, enum list_types dest_list)
{
    if(dest_list == ALIVE_THREADS_LIST) { /* Destination is alive threads list */
        /* Only dead threads can be moved to alive list */
        ASSERT_MSG(thread->state == THREAD_STATE_DEAD,
            "move_thread_to_list_nolock(): Wrong thread state!");

        /* remove thread from the deads */
        xlist_remove_unsafe(&dead_threads_list, &thread->threads_list_node);

        /* set birth state */
        thread->state = THREAD_STATE_BIRTH;

        /* put to alive threads list and tree */
        put_thread_to_list_nolock(thread);
    } else if(dest_list == DEAD_THREADS_LIST) { /* Destination is dead threads list */
        /* Only threads in Death and Birth state can be moved here */
        ASSERT_MSG(thread->state == THREAD_STATE_DEATH ||
                   thread->state == THREAD_STATE_BIRTH,
            "move_thread_to_list_nolock(): Wrong thread state!");

        /* remove thread from list */
        xlist_remove_unsafe(&threads_list, &thread->threads_list_node);
        /* remove thread from tree */
        if(!avl_tree_remove(&threads_tree, thread))
            panic("move_thread_to_list_nolock(): failed to remove thread from tree!\n");

        /* set as dead */
        thread->state = THREAD_STATE_DEAD;

        /* add to deads list */
        xlist_add_last(&dead_threads_list, &thread->threads_list_node);
    }
#ifdef __DEBUG__
    else
        panic("move_thread_to_list_nolock(): wrong list type passed!\n");
#endif
}

/* move thread from one list to another */
static void move_thread_to_list(thread_t *thread, enum list_types dest_list)
{
    unsigned long irqs_state;

    /* acquire lock before calling non-lock version */
    irqs_state = spin_lock_irqsave(&threads_lock);

    /* call to non-lock version */
    move_thread_to_list_nolock(thread, dest_list);

    /* release lock */
    spin_unlock_irqrstor(&threads_lock, irqs_state);
}

/* peeks the head of dead threads list (no lock acquired) */
static thread_t *peek_dead_list_nolock(void)
{
    list_elem_t *item;

    /* peek dead threads list */
    item = xlist_peek_first(&dead_threads_list);

    /* return result */
    if(item)
      return containerof(item, thread_t, threads_list_node);
    else
      return NULL; /* list is empty */
}

/* create new thread structure */
static thread_t *create_thread_struct(void)
{
    thread_t *thread;

    /* allocate memory for new tread struct */
    thread = (thread_t *)kmalloc(sizeof(thread_t));
    if(!thread)
        panic("create_thread_struct(): out of heap memory!\n");

    /* init fields */
    memset(thread, 0, sizeof(thread_t));
    thread->id = get_next_thread_id(); /* Thread ID */
    thread->in_kernel = true; /* initially thread is always executed
                               * at kernel side
                               */

    return thread;
}

/* get not used or create new thread structure */
static thread_t *get_thread_struct(void)
{
    unsigned long irqs_state;
    thread_t *thread;

    /* acquire lock before touching dead threads list */
    irqs_state = spin_lock_irqsave(&threads_lock);

    /* peek first item in list */
    thread = peek_dead_list_nolock();
    /* if exists - move to alive threads list */
    if(thread)
        move_thread_to_list_nolock(thread, ALIVE_THREADS_LIST);
    
    /* release lock */
    spin_unlock_irqrstor(&threads_lock, irqs_state);

    /* if no thread struct selected - create new one */
    if(!thread) {
        thread = create_thread_struct();
        thread->state = THREAD_STATE_BIRTH;
        put_thread_to_list(thread);
    }

    /* executed at kernel side initially */
    thread->in_kernel = true;

    return thread;
}

/* deinit less important fields of thread struct for future reuse */
static void deinit_thread_struct(thread_t *thread)
{
    /* clear thread name */
    if(thread->name != NULL)
        kfree_and_null((void **)&thread->name);

    /* put owning process */
    if(thread->process) {
        proc_put_process(thread->process);
        thread->process = NULL;
    }

    /* reset other fields */
    thread->cpu         = NULL;
    thread->kernel_time = 0;
    thread->user_time   = 0;
    thread->entry       = 0;
    thread->data        = NULL;

    /* TODO: deinit other fields */
}

/* stub function for kernel-side threads */
static int stub_for_kernel_thread(void)
{
    int (*func)(void *data);
    thread_t *thread;
    int retcode;

    /* get current thread */
    thread = thread_get_current_thread();

    /* init at first time */
    if( arch_thread_first_start_init(thread) != NO_ERROR )
        panic("\nFailed to start thread id = %d\n", thread->id);

    /* notify scheduler about new thread start */
    sched_new_thread_started(thread);

    /* call entry and pass user-data into it */
    func = (void *)thread->entry;
    retcode = func(thread->data);

    /* TODO: exit thread stages */
    panic("\nkernel thread termination is not implemented yet!\n");

    return 0;
}

/* stub function for user-side threads */
static int stub_for_user_thread(void)
{
/* TODO: */
    return 0;
}

/* create new kernel stack for thread */
static status_t create_thread_kstack_area(thread_t *thread, const char *name)
{
    status_t err;

    /* init stack adresses to zero for use in error handling */
    thread->kstack_base = thread->kstack_top = 0;

    /* create memory object */
    thread->kstack_id = vm_create_object(name, THREAD_KSTACK_SIZE,
                                         VM_OBJECT_PROTECT_ALL);
    if(thread->kstack_id == VM_INVALID_OBJECTID)
        return ERR_VM_GENERAL;

    /* map created object into kernel address space */
    err = vm_map_object_aligned(vm_get_kernel_aspace_id(), thread->kstack_id,
                        VM_PROT_KERNEL_ALL, KERNEL_STACK_SIZE, &thread->kstack_base);
    /*
     * IMPORTANT NOTE: Alignment of mapped kernel-side stack to its size
     * boundary is very important for thread_get_current_thread() routine.
    */
    if(err != NO_ERROR)
        goto exit_on_error;

    /* compute stack top address */
    thread->kstack_top = thread->kstack_base + THREAD_KSTACK_SIZE - 1;

    /* simulate page fault to actually map all stack pages */
    err = vm_simulate_pf(thread->kstack_base, thread->kstack_top);
    if(err != NO_ERROR)
        goto exit_on_error;

    /* return last success */
    return err;

/* error state */
exit_on_error:
   /* if stack area was mapped - unmap it and delete corresponding
    * memory object
    */
   if(thread->kstack_base != 0) {
       if(vm_unmap_object(vm_get_kernel_aspace_id(), thread->kstack_base) != NO_ERROR)
           panic("create_thread_kstack_area(): failed to recover after error!");
       /* delete memory object created for stack area */
       vm_delete_object(thread->kstack_id);
       /* reset kernel stack area params */
       thread->kstack_id = VM_INVALID_OBJECTID;
       thread->kstack_base = thread->kstack_top = 0;
   }
   
   /* return occured error code */
   return err;
}


/*** Public routines ***/

/* threading init routine */
status_t threading_init(kernel_args_t *kargs, uint curr_cpu)
{
    status_t err;

    if(curr_cpu == BOOTSTRAP_CPU) {
        /* Start threading initialization */

        /* arch-dependend init */
        err = arch_threading_init(kargs);
        if(err != NO_ERROR)
            return err;

        /* process module init */
        err = process_init(kargs);
        if(err != NO_ERROR)
            return err;

        /* scheduling engine init */
        err = scheduler_init(kargs);
        if(err != NO_ERROR)
            return err;

        /* next valid id is 1 */
        next_thread_id = 1;

        /* spinlock for data structures access */
        spin_init(&threads_lock);

        /* threads lists */
        xlist_init(&threads_list);
        xlist_init(&dead_threads_list);

        /* threads tree */
        avl_tree_create( &threads_tree, compare_thread_id,
                         sizeof(thread_t),
                         offsetof(thread_t, threads_tree_node) );
    } else {
      /* TODO: wait for bootstrap cpu completes */
    }

    /* start per CPU initializations */
    err = threading_init_per_cpu(kargs, curr_cpu);
    if(err != NO_ERROR)
        panic("threading_init: initialization for CPU #%d failed!\n", curr_cpu);

    return NO_ERROR;
}

/* per CPU threading init routine */
status_t threading_init_per_cpu(kernel_args_t *kargs, uint curr_cpu)
{
    status_t err;

    /* arch-dependend per cpu init */
    err = arch_threading_init_per_cpu(kargs, curr_cpu);
    if(err != NO_ERROR)
        return err;

    /* process module per cpu init */
    err = process_init_per_cpu(kargs, curr_cpu);
    if(err != NO_ERROR)
        return err;

    /* scheduler per cpu init */
    err = scheduler_init_per_cpu(kargs, curr_cpu);
    if(err != NO_ERROR)
        return err;

    /* create idle thread for this cpu */
    err = thread_continue_as_idle(kargs, curr_cpu);
    if(err != NO_ERROR)
        return err;

    return NO_ERROR;
}

/* continues execution of current instructions flow as idle thread */
status_t thread_continue_as_idle(kernel_args_t *kargs, uint curr_cpu)
{
    char name[SYS_MAX_OS_NAME_LEN];
    thread_t *thread;

    /* allocate new thread structure */
    thread = create_thread_struct();
    if(!thread)
        return ERR_MT_GENERAL;

    /* add to threads list */
    thread->state = THREAD_STATE_BIRTH;
    put_thread_to_list(thread);

    /* locate stack object for this cpu */
    snprintf(name, SYS_MAX_OS_NAME_LEN, "kernel_cpu%d_stack", curr_cpu);
    thread->kstack_id = vm_find_object_by_name(name);
    if(thread->kstack_id == VM_INVALID_OBJECTID)
        goto exit_on_error;

    /* set kernel-side stack */
    thread->kstack_base = kargs->virt_cpu_kstack[curr_cpu].start;
    thread->kstack_top = thread->kstack_base +
                         kargs->virt_cpu_kstack[curr_cpu].size - 1;

    /* set pointer to thread struct into the stack */
    *(thread_t **)(thread->kstack_base) = thread;

    /* set thread name */
    snprintf(name, SYS_MAX_OS_NAME_LEN, "kernel_cpu%d_idle", curr_cpu);
    thread->name = kstrdup(name);
    if(!thread->name)
        goto exit_on_error;

    /* thread owning process */
    thread->process = proc_get_kernel_process();
    /* thread executing on this cpu */
    thread->cpu = &ProcessorSet.processors[curr_cpu];

    /* init arch-specific parts */
    arch_thread_init_struct(thread);

    /* attach to kernel process */
    proc_attach_thread(thread->process, thread);

    /* add to scheduling */
    sched_add_idle_thread(thread, curr_cpu);

    return NO_ERROR; /* all fine */

    /* error occurs */
exit_on_error:
    /* deinit thread structure for reuse in future */
    deinit_thread_struct(thread);

    /* move to deads */
    move_thread_to_list(thread, DEAD_THREADS_LIST);

    return ERR_MT_GENERAL;
}

/* returns current thread structure */
thread_t *thread_get_current_thread(void)
{
    /* pointer to thread struct stored in a bottom
     * of a thread kernel-side stack. So... Just get
     * a pointer to the bottom of current stack and
     * return stored address.
    */
    addr_t stack_ptr = arch_current_stack_pointer();
    return *(thread_t **)(stack_ptr & (~(THREAD_KSTACK_SIZE-1)));
}

/* returns current thread id */
thread_id thread_get_current_thread_id(void)
{
    return thread_get_current_thread()->id;
}

/* check that current thread is a kernel thread */
bool thread_is_kernel_thread(void)
{
    thread_t *th = thread_get_current_thread(); /* get current thread */

    /* compare process ids */
    if (th->process->id == proc_get_kernel_process_id())
        return true;
    else
        return false;
}

/* create new kernel-side thread of execution */
thread_id thread_create_kernel_thread(const char *name, int (*func)(void *data), void *data)
{
    char tmp_name[SYS_MAX_OS_NAME_LEN];
    status_t err;
    thread_t *thread;

    /* get thread struct */
    thread = get_thread_struct();
    if(!thread)
        return INVALID_THREADID;

    /* set thread data */
    thread->name = kstrdup(name);
    thread->process = proc_get_kernel_process();
    thread->cpu = get_current_processor_struct();

    /* create kernel stack area */
    snprintf(tmp_name, SYS_MAX_OS_NAME_LEN, "kernel_thread_%d_kstack", thread->id);
    err = create_thread_kstack_area(thread, tmp_name);
    if(err != NO_ERROR)
        goto exit_on_error;

    /* set pointer to thread struct at the bottom of kernel stack */
    *(thread_t **)(thread->kstack_base) = thread;

    /* init arch-specific parts, before setting other arch-dependend
     * options.
     */
    arch_thread_init_struct(thread);

    /* init kernel stack */
    err = arch_thread_init_kstack(thread, &stub_for_kernel_thread);
    if(err != NO_ERROR)
        goto exit_on_error;

    /* set entry point and user data */
    thread->entry = (addr_t)func;
    thread->data = data;

    /* set default priority and scheduling policy */
    thread->sched_policy.raw = thread->process->def_sched_policy.raw;
    thread->s_prio = thread->process->def_prio;

    /* attach to kernel process */
    proc_attach_thread(thread->process, thread);

    /* add to scheduling scheme */
    sched_add_thread(thread);

    /* return new thread id to caller */
    return thread->id;

  /* if error state */
exit_on_error:
    /* deinit thread structure */
    deinit_thread_struct(thread);

    /* move to deads */
    move_thread_to_list(thread, DEAD_THREADS_LIST);

    /* return error state */
    return INVALID_THREADID;
}

/* transfer control to another thread */
void thread_yield(void)
{
    /* do reschedule for stop current thread
     * and select new one for execution.
     */
    sched_reschedule();
}
