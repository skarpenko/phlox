/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
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
#include <phlox/timer.h>
#include <phlox/scheduler.h>
#include <phlox/process.h>
#include <phlox/thread.h>
#include <phlox/thread_private.h>


/* Redefinition for convenience */
typedef xlist_t threads_list_t;

/* Thread lists types */
enum list_types {
    ALIVE_THREADS_LIST,  /* List of alive threads */
    DEATH_THREADS_LIST,  /* List of threads in death state */
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

/* List of threads in transition to dead state.
 * Valid thread states in that list is Running and Death.
 * Threads in death state will be moved to dead list.
*/
static threads_list_t death_threads_list;

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
    retval = (thread_id)atomic_inc_ret((atomic_t*)&next_thread_id);
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
    switch(dest_list) {
      /* Destination is alive threads list */
      case ALIVE_THREADS_LIST:
        /* Only dead threads can be moved to alive list */
        if(thread->state != THREAD_STATE_DEAD)
            panic("move_thread_to_list_nolock(): Wrong thread state!");

        /* remove thread from the deads */
        xlist_remove_unsafe(&dead_threads_list, &thread->threads_list_node);

        /* set birth state */
        thread->state = THREAD_STATE_BIRTH;

        /* put to alive threads list and tree */
        put_thread_to_list_nolock(thread);
        break;

      /* Destination is death threads list */
      case DEATH_THREADS_LIST:
        /* Only threads in Running state can be moved here */
        if(thread->state != THREAD_STATE_RUNNING)
            panic("move_thread_to_list_nolock(): Wrong thread state!");

        /* remove thread from list of active threads */
        xlist_remove_unsafe(&threads_list, &thread->threads_list_node);
        /* remove thread from tree */
        if(!avl_tree_remove(&threads_tree, thread))
            panic("move_thread_to_list_nolock(): failed to remove thread from tree!\n");

        /* Thread is in Running state, we can't switch state to Death here,
         * so... we set next state, after reschedule state will be correct.
         */
        thread->next_state = THREAD_STATE_DEATH;

        /* add to death list */
        xlist_add_last(&death_threads_list, &thread->threads_list_node);
        break;

      /* Destination is dead threads list */
      case DEAD_THREADS_LIST:
        /* Threads in Birth state contained in active threads list and tree,
         * the ones in Death state contained only in death treads list.
         */
        if(thread->state == THREAD_STATE_BIRTH) {
            /* remove thread from active threads list */
            xlist_remove_unsafe(&threads_list, &thread->threads_list_node);
            /* remove thread from tree */
            if(!avl_tree_remove(&threads_tree, thread))
                panic("move_thread_to_list_nolock(): failed to remove thread from tree!\n");
        } else if(thread->state == THREAD_STATE_DEATH) {
            /* remove thread from death list */
            xlist_remove_unsafe(&death_threads_list, &thread->threads_list_node);
        } else /* Only threads in Death and Birth state can be moved here */
            panic("move_thread_to_list_nolock(): Wrong thread state!");

        /* set as dead */
        thread->state = THREAD_STATE_DEAD;

        /* add to deads list */
        xlist_add_last(&dead_threads_list, &thread->threads_list_node);
        break;

      /* panic if wrong state */
      default:
        panic("move_thread_to_list_nolock(): wrong list type passed!\n");
    }
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
    spin_init_locked(&thread->lock); /* thread structure initially locked */

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

    /* set lock into locked state, so... on reuse the structure
     * will be initially locked.
     */
    spin_init_locked(&thread->lock);

    /* TODO: deinit other fields */

    ASSERT_MSG(
        thread->term_cbs_list.first == NULL &&
        thread->term_cbs_list.last == NULL,
        "deinit_thread_struct(): termination callbacks list is not empty."
    );
}

/* moves threads in death state from death list to dead list */
static void purge_death_threads_list_nolock(void)
{
    list_elem_t *iter = xlist_peek_first(&death_threads_list);
    thread_t *thread;

    while(iter != NULL) {
        /* get thread structure */
        thread = containerof(iter, thread_t, threads_list_node);

        /* move iterator to next item */
        iter = iter->next;

        /* move to dead list if in death state */
        if(thread->state == THREAD_STATE_DEATH) {
            /* deinit structure ... */
            deinit_thread_struct(thread);
            /* ... and move to deads */
            move_thread_to_list_nolock(thread, DEAD_THREADS_LIST);
        }
    }
}

/* get not used or create new thread structure */
static thread_t *get_thread_struct(void)
{
    unsigned long irqs_state;
    thread_t *thread;

    /* acquire lock before touching dead threads list */
    irqs_state = spin_lock_irqsave(&threads_lock);

    /* move threads in death state to deads list */
    purge_death_threads_list_nolock();

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
    sched_complete_context_switch();

    /* call entry and pass user-data into it */
    func = (void *)thread->entry;
    retcode = func(thread->data);

    /* call thread exit routine */
    thread_exit(retcode);

    /*** Control never goes here ***/

    return 0;
}

/* stub function for user-side threads */
static int stub_for_user_thread(void)
{
    thread_t *thread;

    /* get current thread */
    thread = thread_get_current_thread();

    /* init at first time */
    if( arch_thread_first_start_init(thread) != NO_ERROR )
        panic("\nFailed to start thread id = %d\n", thread->id);

    /* notify scheduler about new thread start */
    sched_complete_context_switch();

    /* pass control to user-space code */
    arch_thread_enter_uspace(thread);

    /*** Control never goes here ***/

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
    thread->kstack_top = thread->kstack_base + THREAD_KSTACK_SIZE;

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

/* process termination callbacks registered for thread */
static void purge_term_cbs_list(thread_t *th)
{
    unsigned long irqs_state;
    list_elem_t *le;
    thread_cbd_t *cbd;

    do {
        /* lock thread */
        irqs_state = spin_lock_irqsave(&th->lock);

        /* extract callback descriptor */
        le = xlist_extract_first(&th->term_cbs_list);
        cbd = (le) ? containerof(le, thread_cbd_t, list_node) : NULL;

        /* release thread lock */
        spin_unlock_irqrstor(&th->lock, irqs_state);

        /* call the callback function */
        if(cbd != NULL) cbd->func(cbd);

    } while(cbd != NULL);
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
        xlist_init(&death_threads_list);

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

    /* unlock thread struct */
    thread_unlock_thread(thread);

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

/* return structure for specified thread */
thread_t *thread_get_thread_struct(thread_id tid)
{
    thread_t *look_for, *thread;
    unsigned long irqs_state;

    /* init data for search */
    look_for = containerof(&tid, thread_t, id);

    /* lock threads containers before touching */
    irqs_state = spin_lock_irqsave(&threads_lock);

    /* search for thread */
    thread = avl_tree_find(&threads_tree, look_for, NULL);

    /* release lock */
    spin_unlock_irqrstor(&threads_lock, irqs_state);

    return thread;
}

/* get struct for specified thread with lock acquired */
thread_t *thread_get_thread_struct_locked(thread_id tid)
{
    thread_t *thread = thread_get_thread_struct(tid);
    if(thread)
        thread_lock_thread(thread);
    return thread;
}

/* get current thread struct with lock acquired */
thread_t *thread_get_current_thread_locked(void)
{
    thread_t *thread = thread_get_current_thread();
    thread_lock_thread(thread);
    return thread;
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
thread_id thread_create_kernel_thread(const char *name, int (*func)(void *data),
    void *data, bool suspended)
{
    char tmp_name[SYS_MAX_OS_NAME_LEN];
    status_t err;
    thread_t *thread;

    /* get thread struct */
    thread = get_thread_struct();
    if(!thread)
        return INVALID_THREADID;

    /* set thread data */
    thread->name = (name ? kstrdup(name) : NULL);
    if(name && !thread->name) goto exit_on_error;
    thread->process = proc_get_kernel_process();
    thread->cpu = get_current_processor_struct();

    /* create kernel stack area, if was not created before */
    if(thread->kstack_id == VM_INVALID_OBJECTID) {
        snprintf(tmp_name, SYS_MAX_OS_NAME_LEN, "kernel_thread_%d_kstack", thread->id);
        err = create_thread_kstack_area(thread, tmp_name);
        if(err != NO_ERROR)
            goto exit_on_error;
    }

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

    /* add to scheduling scheme if creating in
     * not suspended state initially, or set proper
     * state and skip adding to scheduling.
     */
    if(suspended) {
        thread->state = THREAD_STATE_SUSPENDED;
        thread->next_state = THREAD_STATE_READY;
    } else
       sched_add_thread(thread);

    /* unlock thread struct */
    thread_unlock_thread(thread);

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

/* create new user-side thread of execution */
thread_id thread_create_user_thread(const char *name, process_t *proc, addr_t entry,
    void *data, object_id stack_obj, addr_t stack_base, bool suspended)
{
    char tmp_name[SYS_MAX_OS_NAME_LEN];
    status_t err;
    vm_object_t *stack_obj_dat;
    aspace_id proc_aid;
    size_t stack_size;
    thread_t *thread;

    /* ensure that not a kernel process passed */
    if(proc == proc_get_kernel_process())
        return INVALID_THREADID;

    /* check that owning process is in correct state */
    if(proc->state != PROCESS_STATE_BIRTH && proc->state != PROCESS_STATE_NORMAL)
        return INVALID_THREADID;

    /* get process address space id */
    proc_aid = proc_get_aspace_id(proc);
    if(proc_aid == VM_INVALID_ASPACEID)
        return INVALID_THREADID;

    /* get stack object data */
    stack_obj_dat = vm_get_object_by_id(stack_obj);
    if(!stack_obj_dat)
        return INVALID_THREADID;

    /* stack size */
    stack_size = stack_obj_dat->size;

    /* if stack base address was specified than map stack using it otherwise let
     * VM subsystem choose.
     */
    if(stack_base)
        err = vm_map_object_exactly(proc_aid, stack_obj, VM_PROT_USER_DEFAULT, stack_base);
    else
        err = vm_map_object(proc_aid, stack_obj, VM_PROT_USER_DEFAULT, &stack_base);

    if(err != NO_ERROR)
        goto exit_on_error;

    /* get thread struct */
    thread = get_thread_struct();
    if(!thread)
        goto exit_on_error_unmap;

    /* set thread data */
    thread->name = (name ? kstrdup(name) : NULL);
    if(name && !thread->name) goto exit_on_error_unmap;
    thread->cpu = get_current_processor_struct();
    thread->process = proc_inc_refcnt(proc);

    /* create kernel stack area, if was not created before */
    if(thread->kstack_id == VM_INVALID_OBJECTID) {
        snprintf(tmp_name, SYS_MAX_OS_NAME_LEN, "user_thread_%d_kstack", thread->id);
        err = create_thread_kstack_area(thread, tmp_name);
        if(err != NO_ERROR)
            goto exit_on_error_proc_detach;
    }

    /* set pointer to thread struct at the bottom of kernel stack */
    *(thread_t **)(thread->kstack_base) = thread;

    /* set user space stack */
    thread->ustack_id   = stack_obj;
    thread->ustack_base = stack_base;
    thread->ustack_top  = stack_base + stack_size;

    /* init arch-specific parts, before setting other arch-dependend
     * options.
     */
    arch_thread_init_struct(thread);

    /* init kernel stack */
    err = arch_thread_init_kstack(thread, &stub_for_user_thread);
    if(err != NO_ERROR)
        goto exit_on_error_proc_detach;

    /* set entry point and user data */
    thread->entry = entry;
    thread->data = data;

    /* set default priority and scheduling policy */
    thread->sched_policy.raw = thread->process->def_sched_policy.raw;
    thread->s_prio = thread->process->def_prio;

    /* attach to kernel process */
    proc_attach_thread(thread->process, thread);

    /* switch process state to normal if main thread was added */
    if(thread->process->state == PROCESS_STATE_BIRTH) {
        thread->process->main = thread;
        thread->process->state = PROCESS_STATE_NORMAL;
    }

    /* add to scheduling scheme if creating in
     * not suspended state initially, or set proper
     * state and skip adding to scheduling.
     */
    if(suspended) {
        thread->state = THREAD_STATE_SUSPENDED;
        thread->next_state = THREAD_STATE_READY;
    } else
       sched_add_thread(thread);

    /* unlock thread struct */
    thread_unlock_thread(thread);

    /* return new thread id to caller */
    return thread->id;

/* if error state */
exit_on_error_proc_detach:
    proc_put_process(thread->process);
    thread->process = NULL;

    /* deinit thread structure */
    deinit_thread_struct(thread);

    /* move to deads */
    move_thread_to_list(thread, DEAD_THREADS_LIST);

exit_on_error_unmap:
    err = vm_unmap_object(proc_aid, stack_base);
    if(err != NO_ERROR)
        panic("thread_create_user_thread(): failed to unmap stack!\n");

exit_on_error:
    vm_put_object(stack_obj_dat);


    /* return error state */
    return INVALID_THREADID;
}

/* transfer control to another thread */
void thread_yield(void)
{
    /* before we proceed - disable interrupts and acquire
     * scheduling lock.
     */
     local_irqs_disable();
     sched_lock_acquire();

    /* perform rescheduling, stop current thread
     * and select new one for execution.
     * this also enables local interrupts and releases
     * scheduling lock.
     */
    sched_reschedule();
}

/* terminate current thread with specified exit code */
void thread_exit(int exitcode)
{
    thread_t *thread = thread_get_current_thread();
    process_t *process = thread->process;

    /* check current state */
    if(is_kernel_start_stage(K_KERNEL_STARTUP) || local_irqs_disabled())
        panic("thread_exit(): called during kernel startup or irqs disabled!");

    /* TODO: Possible algorithm here is the following:
     *   1. Boost thread priority.
     *   2. Kill alarms
     *   3. Kill userspace regions
     *   4. If not a kernel thread:
     *     4.1. Move thread to kernel process
     *     4.2. If main thread: mark process to destroy
     *     4.3. Switch to kernel address space
     *   5. Delete process if needed
     *   6. Wake up thread termination waiters
     * If needed to destroy kernel-side stack area
     *   7. Switch to temp stack and call thread_exit2 using
     *      switch_stack_and_call.
     *
     * In thread_exit2:
     *   1. Remove thread from kernel process
     *   2. Free other resources
     *   3. Set DEATH state
     *   4. Reschedule
    */

    /* before proceed with thread termination,
     * call all registered termination callbacks.
    */
    purge_term_cbs_list(thread);

    /* disable irqs on this cpu */
    local_irqs_disable();

    /* lock thread before */
    thread_lock_thread(thread);

    /* detach thread from process */
    proc_detach_thread(process, thread);
    /* move to death list */
    move_thread_to_list(thread, DEATH_THREADS_LIST);

    /* set next state for thread and reschedule */
    thread->next_state = THREAD_STATE_DEATH;

    /* unlock and call reschedule */
    thread_unlock_thread(thread);
    sched_lock_acquire();
    sched_reschedule();
    /* NOTE: interrupts will be enabled during rescheduling
     *       as far as scheduling lock will be released.
     */

    /** control never goes here **/
}

/* suspend currently running thread */
status_t thread_suspend_current(void)
{
    thread_t *thread;

    /* disable interrupts and select current thread */
    local_irqs_disable();
    thread = thread_get_current_thread_locked();

    /* set next state as suspended */
    if(!IS_THREAD_NEXSTATE_READY(thread) && !IS_THREAD_NEXSTATE_RUNNING(thread)) {
        /* cannot suspend - return an error */
        thread_unlock_thread(thread);
        local_irqs_enable();
        return ERR_MT_INVALID_STATE;
    } else
        thread->next_state = THREAD_STATE_SUSPENDED;

    /* unlock thread and reschedule */
    thread_unlock_thread(thread);
    sched_lock_acquire();
    sched_reschedule();
    /* NOTE: interrupts will be reenabled during rescheduling and scheduling lock
     *       will be released.
     */

    return NO_ERROR;
}

/* suspend specified thread */
status_t thread_suspend(thread_id tid)
{
    /* get thread data */
    thread_t *thread = thread_get_thread_struct(tid);
    if(thread == NULL)
        return ERR_MT_INVALID_HANDLE;

    /* disable interrupts on this cpu and lock thread */
    local_irqs_disable();
    thread_lock_thread(thread);

    /* change next state for thread only if it is running or
     * ready to run at that time.
     */
    if((!IS_THREAD_STATE_READY(thread) && !IS_THREAD_STATE_RUNNING(thread)) ||
       (!IS_THREAD_NEXSTATE_READY(thread) && !IS_THREAD_NEXSTATE_RUNNING(thread)))
    {
        /* cannot suspend thread - return an error */
        thread_unlock_thread(thread);
        local_irqs_enable();
        return ERR_MT_INVALID_STATE;
    } else
        thread->next_state = THREAD_STATE_SUSPENDED;

    /* unlock thread */
    thread_unlock_thread(thread);

    /* check if suspending currently running thread
     * if true - reschedule immediately, interrupts will be
     * reenabled during rescheduling.
    */
    if(thread_get_current_thread_id() == tid) {
        sched_lock_acquire();
        sched_reschedule();
    } else
        local_irqs_enable();

    return NO_ERROR;
}

/* resume specified thread */
status_t thread_resume(thread_id tid)
{
    /* get thread data */
    thread_t *thread = thread_get_thread_struct(tid);
    if(thread == NULL)
        return ERR_MT_INVALID_HANDLE;

    /* disable interrupts on this cpu and lock thread */
    local_irqs_disable();
    thread_lock_thread(thread);

    /* wake up only suspended threads */
    if(thread->state == THREAD_STATE_SUSPENDED)
        sched_add_thread(thread);

    /* unlock thread and enable interrupts again */
    thread_unlock_thread(thread);
    local_irqs_enable();

    return NO_ERROR;
}

/* put current thread into sleep */
void thread_sleep(uint msec)
{
    thread_t *thread;

    /* just switch to another thread if time
     * was not specified
     */
    if(!msec) {
        thread_yield();
        return;
    }

    /* disable interrupts and take current thread */
    local_irqs_disable();
    thread = thread_get_current_thread_locked();

    /* add thread to event queue of the timer */
    if(IS_THREAD_NEXSTATE_READY(thread) || IS_THREAD_NEXSTATE_RUNNING(thread))
        timer_lull_thread(thread, TIMER_MSEC_TO_TICKS(msec));

    /* unlock thread and reschedule */
    thread_unlock_thread(thread);
    sched_lock_acquire();
    sched_reschedule();
    /* NOTE: interrupts will be reenabled during rescheduling and scheduling lock
     *       will be released.
     */
}

/* put thread to sleep by its id */
status_t thread_sleep_id(thread_id tid, uint msec)
{
    thread_t *thread;

    /* if id of the current thread specified */
    if(thread_get_current_thread_id() == tid) {
        thread_sleep(msec);
        return NO_ERROR;
    }

    /* get thread data */
    thread = thread_get_thread_struct(tid);
    if(thread == NULL)
        return ERR_MT_INVALID_HANDLE;

    /* disable interrupts and lock thread */
    local_irqs_disable();
    thread_lock_thread(thread);

    /* check thread state is correct */
    if((!IS_THREAD_STATE_READY(thread) && !IS_THREAD_STATE_RUNNING(thread)) ||
       (!IS_THREAD_NEXSTATE_READY(thread) && !IS_THREAD_NEXSTATE_RUNNING(thread)))
    {
        /* cannot put thread to bed - unlock thread,
         * enable irqs and return error
         */
        thread_unlock_thread(thread);
        local_irqs_enable();
        return ERR_MT_INVALID_STATE;
    }

    /* remove thread from runqueue
     * if it is not currently running
     */
    if(thread->state == THREAD_STATE_READY) {
        sched_remove_thread(thread);
    }

    /* add thread to event queue of the timer */
    timer_lull_thread(thread, TIMER_MSEC_TO_TICKS(msec));

    /* unlock thread and restore irqs */
    thread_unlock_thread(thread);
    local_irqs_enable();

    return NO_ERROR;
}

/* register termination callback */
thread_cbd_t *thread_register_term_cb(thread_cbd_t *cbd)
{
    ASSERT_MSG(cbd->thread->lock != 0,
        "thread_register_term_cb(): thread was not locked.");

    /* put callback to callbacks list of the thread */
    xlist_add_last(&cbd->thread->term_cbs_list, &cbd->list_node);

    return cbd;
}

/* unregister termination callback */
void thread_unregister_term_cb(thread_cbd_t *cbd)
{
    ASSERT_MSG(cbd->thread->lock != 0,
        "thread_unregister_term_cb(): thread was not locked.");

    /* remove callback from the callbacks list */
    xlist_remove_unsafe(&cbd->thread->term_cbs_list, &cbd->list_node);
}

/* return process data */
process_t *thread_get_process(thread_t *thread)
{
    process_t *proc;

    /* disable interrupts on this cpu and lock thread */
    local_irqs_disable();
    thread_lock_thread(thread);

    proc = thread_get_process_nolock(thread);
    
    /* unlock thread and restore irqs */
    thread_unlock_thread(thread);
    local_irqs_enable();

    return proc;
}

/* return process data without locking thread */
process_t *thread_get_process_nolock(thread_t *thread)
{
    ASSERT_MSG(thread->lock != 0, "thread_get_process_nolock(): thread was not locked.");
    return (thread->process) ? proc_inc_refcnt(thread->process) : NULL;
}

/* return process id */
proc_id thread_get_process_id(thread_t *thread)
{
    proc_id id = INVALID_PROCESSID;

    /* disable interrupts on this cpu and lock thread */
    local_irqs_disable();
    thread_lock_thread(thread);

    if(thread->process)
        id = thread->process->id;

    /* unlock thread and restore irqs */
    thread_unlock_thread(thread);
    local_irqs_enable();

    return id;
}
