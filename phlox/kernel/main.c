/*
* Copyright 2007-2010, Stepan V.Karpenko. All rights reserved.
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
#include <phlox/thread.h>
#include <phlox/klog.h>
#include <phlox/debug.h>
#include <boot/bootfs.h>


/* Global kernel args */
kernel_args_t globalKargs;

/* Current kernel start up stage */
static int _kernel_start_stage = K_KERNEL_STARTUP;

void print_kernel_memory_map(void); /* for DEBUG only */
void init_console_writer(void); /* for DEBUG only */

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
    }

    /* init console writer */
    init_console_writer();

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

/* threads controller routine */
int thread_ctl(void *data)
{
    thread_id me = thread_get_current_thread_id();

    kprint("Threads controller (id = %d): started...\n", me);

    kprint("Threads controller: Wait for 3000 msec before output start...\n");
    thread_sleep(3000);

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
