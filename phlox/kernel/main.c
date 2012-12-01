/*
* Copyright 2007-2012, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <stdarg.h>
#include <arch/cpu.h>
#include <phlox/errors.h>
#include <phlox/types.h>
#include <phlox/kernel.h>
#include <phlox/processor.h>
#include <phlox/interrupt.h>
#include <phlox/machine.h>
#include <phlox/timer.h>
#include <phlox/kargs.h>
#include <phlox/vm.h>
#include <phlox/heap.h>
#include <phlox/thread.h>
#include <phlox/sem.h>
#include <phlox/klog.h>
#include <phlox/debug.h>
#include <boot/bootfs.h>


/* Global kernel args */
kernel_args_t globalKargs;

/* Current kernel start up stage */
static int _kernel_start_stage = K_KERNEL_STARTUP;

void print_kernel_memory_map(void); /* for DEBUG only */

/* for threading DEBUG */
vuint thread0_ctr = 0;
vuint thread1_ctr = 0;
vuint thread2_ctr = 0;
vuint thread_ctl_ctr = 0;
void wait(long msec);
int thread0(void *data);
int thread1(void *data);
int thread2(void *data);
int thread_ctl(void *data);
void timeout_call(timeout_id id, void *data);
int thread_sem0(void *data);
int thread_sem1(void *data);
int thread_sem2(void *data);
int thread_sem3(void *data);


/* Main entry point on kernel start */
void _phlox_kernel_entry(kernel_args_t *kargs, uint num_cpu);  /* keep compiler happy */
void _phlox_kernel_entry(kernel_args_t *kargs, uint num_cpu)
{
    status_t err;

    /* if we are bootstrap processor,
     * store kernel args to global variable and
     * init kernel logging.
     */
    if(num_cpu==0) {
      /* init kernel log first */
      klog_init(kargs);

      /* kernel debugging facilities init */
      debug_init(kargs);

      /* copy kernel args to global variable */
      memcpy(&globalKargs, kargs, sizeof(kernel_args_t));
      /* verify */
      if(globalKargs.magic != KARGS_MAGIC)
          panic("Kernel args damaged or incorrect!\n");

      /* put hello into klog */
      kprint("\nWelcome to Phlox Kernel! (built at: %s %s)\n", __DATE__, __TIME__);
    } else {
       /* wait until BSP completes its job? */
    }


    /* processor set initialization */
    processor_set_init(&globalKargs, num_cpu);


    /* initialization stages performed by bootstrap
     * processor only, others must wait until all
     * initialization stages completes.
     */
    if(num_cpu==0) {
       /* init machine-specific interfaces */
       machine_init(&globalKargs);

       /* init interrupt handling */
       interrupt_init(&globalKargs);

       /* init virtual memory manager */
       vm_init(&globalKargs);

       /* init console */
       debug_init_console_output(&globalKargs);

       /* system timer init */
       timer_init(&globalKargs);
    } else {
       /* wait until BSP completes? */
    }


    /* processor set initialization continue */
    processor_set_init_after_vm(&globalKargs, num_cpu);

    /* threading init */
    err = threading_init(&globalKargs, num_cpu);
    if(err != NO_ERROR)
        panic("Threading initialization stage failed!\n");

    /* continue initialization of modules that requires threading.
     * initialization goes only on bootstrap processor, others - waiting.
    */
    if(num_cpu==0) {
        /* continue VM init */
        err = vm_init_post_threading(&globalKargs);
        if(err != NO_ERROR)
            panic("VM post-threading init failed!\n");

        /* init heap */
        err = heap_init_postthread(&globalKargs);
        if(err != NO_ERROR)
            panic("Heap post-threading init failed!\n");

        /* init timer module */
        err = timer_init_after_threading(&globalKargs);
        if(err != NO_ERROR)
            panic("Timer threading dependent part initialization failed!\n");

        /* init semaphores module */
        err = semaphores_init(&globalKargs);
        if(err != NO_ERROR)
            panic("Semaphores initialization failed!\n");

        /* continue VM init */
        err = vm_init_post_sema(&globalKargs);
        if(err != NO_ERROR)
            panic("VM post-semaphores init failed!\n");
    } else {
       /* wait until BSP completes? */
    }


    /* page fault handling test */
    {
        object_id id;
        aspace_id kid = vm_get_kernel_aspace_id();
        addr_t vaddr = 0;

        /* create new object */
        id = vm_create_object("for_page_fault_test", PAGE_SIZE, VM_OBJECT_PROTECT_ALL);

        /* ... and map it */
        vm_map_object(kid, id, VM_PROT_KERNEL_ALL, &vaddr);

        kprint("\nPage fault test (touching address 0x%x)...", vaddr);
        *((int *)vaddr) = 0; /* this initiates page fault */
        kprint("PASSED\n");
    }

    /* kernel's memory map */
    print_kernel_memory_map();

    /* current thread */
    kprint("\nCurrent thread: id = %d, name = %s\n",
        thread_get_current_thread()->id,
        thread_get_current_thread()->name);

    /* threading test */
    {
        thread_id tid;
        /* thread 0 */
        tid = thread_create_kernel_thread("kernel_thread0", &thread0, NULL, false);
        if(tid == INVALID_THREADID) kprint("Failed to create thread0!\n");
        /* thread 1 */
        tid = thread_create_kernel_thread("kernel_thread1", &thread1, (void *)tid, false);
        if(tid == INVALID_THREADID) kprint("Failed to create thread1!\n");
        /* thread 2 */
        tid = thread_create_kernel_thread("kernel_thread2", &thread2, (void *)tid, false);
        if(tid == INVALID_THREADID) kprint("Failed to create thread2!\n");
        /* threads controller */
        tid = thread_create_kernel_thread("kernel_thread_ctl", &thread_ctl, NULL, false);
        if(tid == INVALID_THREADID) kprint("Failed to create thread_ctl!\n");
        /* semaphores test */
        tid = thread_create_kernel_thread("kernel_thread_sem0", &thread_sem0, NULL, false);
        if(tid == INVALID_THREADID) kprint("Failed to create thread_sem0!\n");
    }

    /* init console writer */
    debug_init_console_writer();

    /* switch to next kernel start stage */
    _kernel_start_stage = K_SERVICES_STARTUP;

    /* enable interrupts */
    local_irqs_enable();

    /* start idle cycle for this cpu. wait for reschedule. */
    processor_idle_cycle();
}

/* get current kernel start up stage */
int kernel_start_stage(void)
{
    return _kernel_start_stage;
}

/* returns true if specified stage is current */
bool is_kernel_start_stage(int stage)
{
    return (_kernel_start_stage == stage);
}

/* returns true if specified stage was completed */
bool is_kernel_start_stage_compl(int stage)
{
    return (_kernel_start_stage > stage);
}


/*
 * Prints kernel's memory map.
 * used for debug and will be removed
 */
void print_kernel_memory_map(void)
{
    vm_address_space_t *aspace = vm_get_kernel_aspace();
    vm_mapping_t *mapping;
    list_elem_t *item;
    int i = 0;

    kprint("\nKernel address space:\n");

    item = xlist_peek_first(&aspace->mmap.mappings_list);
    while(item != NULL) {
        mapping = containerof(item, vm_mapping_t, list_node);
        kprint("mapping %2d: %x - %x ", ++i, mapping->start, mapping->end);
        if(mapping->type != VM_MAPPING_TYPE_HOLE) {
           kprint("Object -> %s\n", mapping->object->name);
        } else
            kprint("Hole\n");

        item = xlist_peek_next(item);
    }

    vm_put_aspace(aspace);
}

/* thread 0 routine */
int thread0(void *data)
{
    thread_id me = thread_get_current_thread_id();

    kprint("Thread0 (id = %d): started...\n", me);

    kprint("Thread0: Wait for 5000 msec before start...\n");
    thread_sleep(5000);

    while(1) {
        thread0_ctr++;
        /* wait a little */
        thread_sleep(50);
    }
}

/* thread 1 routine */
int thread1(void *data)
{
    thread_id me = thread_get_current_thread_id();
    thread_id slave = (thread_id)data;
    bool slave_active = true;
    bigtime_t curr_time;
    bigtime_t time = timer_get_time();
    bigtime_t self_suspend = time;

    kprint("Thread1 (id = %d): started...\n", me);

    kprint("Thread1: Wait for 7000 msec before start...\n");
    thread_sleep(7000);

    while(1) {
        thread1_ctr++;

        /* current time */
        curr_time = timer_get_time();

        /* simulate slave thread controlling */
        if(curr_time-time > 5000) {

           if(slave_active) {
              slave_active = false;
              /* put slave into sleep periodically */
              if(thread_sleep_id(slave, 8333)) {
                  kprint("Thread1: Failed to put slave into sleep!\n");
              } else {
                  kprint("Thread1: Slave goes to sleep!\n");
              }
              /** thread_suspend(slave); */
           } else {
              slave_active = true;
              /** thread_resume(slave); */
           }

           time = timer_get_time();
        }

        /* suspend current thread periodically */
        if(curr_time-self_suspend > 22000) {
            thread_suspend_current();
            self_suspend = timer_get_time();
        }

        /* wait a little */
        thread_sleep(50);
    }
}
/* thread 2 routine */
int thread2(void *data)
{
    thread_id me = thread_get_current_thread_id();
    thread_id slave = (thread_id)data;
    bigtime_t time = timer_get_time();

    kprint("Thread2 (id = %d): started...\n", me);

    kprint("Thread2: Wait for 9000 msec before start...\n");
    thread_sleep(9000);

    while(1) {
        thread2_ctr++;

        /* wake up our slave thread periodically */
        if(timer_get_time()-time > 40000) {
            thread_resume(slave);
            time = timer_get_time();
        }

        /* wait a little */
        thread_sleep(50);
    }
}

/* timeout call routine */
void timeout_call(timeout_id id, void *data)
{
    kprint("TIMEOUT CALL id = %d\n", id);
    /* re-register call */
    timer_timeout_sched(timeout_call, NULL, TIMER_MSEC_TO_TICKS(1000));
}

/* threads controller routine */
int thread_ctl(void *data)
{
    thread_id me = thread_get_current_thread_id();
    timeout_id t_id;

    kprint("Threads controller (id = %d): started...\n", me);

    kprint("Threads controller: Wait for 20000 msec before start...\n");
    thread_sleep(20000);

    /* register timeout call */
    t_id = timer_timeout_sched(timeout_call, NULL, TIMER_MSEC_TO_TICKS(1000));
    /* timer_timeout_cancel(t_id); */

    while(1) {
        thread_ctl_ctr++;
        if( !(thread_ctl_ctr % 10) ) {
            kprint("Threads controller message: ctr0 = %u  ctr1 = %u  ctr2 = %u\n",
                thread0_ctr, thread1_ctr, thread2_ctr);
        }
        /* pass control to next thread */
        thread_yield();
    }
}

/* semaphores test. thread 0 */
int thread_sem0(void *data)
{
    thread_id me = thread_get_current_thread_id();
    thread_id tid;
    sem_id sem;

    kprint("thread_sem0: started id = %d\n", me);

    sem = sem_create("sem_test", 2, 0);
    if(sem != INVALID_SEMID)
        kprint("thread_sem0: semaphore created id = %d\n", sem);
    else
        panic("thread_sem0: FAILED to create semaphore!\n");

    /* thread1 */
    tid = thread_create_kernel_thread("kernel_thread_sem1", &thread_sem1, NULL, false);
    if(tid == INVALID_THREADID) kprint("Failed to create thread_sem1!\n");

    /* thread2 */
    tid = thread_create_kernel_thread("kernel_thread_sem2", &thread_sem2, NULL, false);
    if(tid == INVALID_THREADID) kprint("Failed to create thread_sem2!\n");

    /* thread3 */
    tid = thread_create_kernel_thread("kernel_thread_sem3", &thread_sem3, NULL, false);
    if(tid == INVALID_THREADID) kprint("Failed to create thread_sem3!\n");

    /* wait */
    thread_sleep(3000);

    /* wake up threads */
    sem_up(sem, 2);
/*    sem_delete(sem); */

    return 0;
}

/* semaphores test. thread 1 */
int thread_sem1(void *data)
{
    thread_id me = thread_get_current_thread_id();
    sem_id sem;
    status_t err;

    kprint("thread_sem1: started id = %d\n", me);

    sem = sem_get_by_name("sem_test");
    if(sem != INVALID_SEMID)
        kprint("thread_sem1: semaphore found!\n");
    else
        kprint("thread_sem1: semaphore not found!\n");

    kprint("thread_sem1: wait for semaphore acquire\n");
    err = sem_down_ex(sem, 1, 2000, SEMF_TIMEOUT);
    kprint("thread_sem1: semaphore acquired, err = 0x%x\n", err);

    return 0;
}

/* semaphores test. thread 2 */
int thread_sem2(void *data)
{
    thread_id me = thread_get_current_thread_id();
    sem_id sem;
    status_t err;

    kprint("thread_sem2: started id = %d\n", me);

    sem = sem_get_by_name("sem_test");
    if(sem != INVALID_SEMID)
        kprint("thread_sem2: semaphore found!\n");
    else
        kprint("thread_sem2: semaphore not found!\n");

    kprint("thread_sem2: wait for semaphore acquire\n");
    err = sem_down(sem, 1);
    kprint("thread_sem2: semaphore acquired, err = 0x%x\n", err);

    return 0;
}

/* semaphores test. thread 3 */
int thread_sem3(void *data)
{
    thread_id me = thread_get_current_thread_id();
    sem_id sem;
    status_t err;

    kprint("thread_sem3: started id = %d\n", me);

    sem = sem_get_by_name("sem_test");
    if(sem != INVALID_SEMID)
        kprint("thread_sem3: semaphore found!\n");
    else
        kprint("thread_sem3: semaphore not found!\n");

    kprint("thread_sem3: wait for semaphore acquire\n");
    err = sem_down(sem, 1);
    kprint("thread_sem3: semaphore acquired, err = 0x%x\n", err);

    return 0;
}
