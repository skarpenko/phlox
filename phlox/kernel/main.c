/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
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
#include <phlox/process.h>
#include <phlox/thread.h>
#include <phlox/sem.h>
#include <phlox/klog.h>
#include <phlox/debug.h>
#include <phlox/imgload.h>


#define PRINT_KERNEL_MMAP 0

/* Path to init service binary */
#define INIT_SERVICE "service/init.elf"

/* Global kernel args */
kernel_args_t globalKargs;

/* Current kernel start up stage */
static int _kernel_start_stage = K_KERNEL_STARTUP;

void print_kernel_memory_map(void); /* for DEBUG only */

/* system init thread */
static int init_thread(void *data);


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

#if PRINT_KERNEL_MMAP==1
    /* kernel memory map */
    print_kernel_memory_map();
#endif

    /* current thread */
    kprint("\nCurrent thread: id = %d, name = %s\n",
        thread_get_current_thread()->id,
        thread_get_current_thread()->name);

    /* create system initialization thread */
    {
        thread_id tid;
        tid = thread_create_kernel_thread("kernel_init_thread", &init_thread, NULL, false);
        if(tid == INVALID_THREADID)
            panic("Failed to create system initialization thread.");
    }

    /* init console writer */
    debug_init_console_writer();

    /* switch to next kernel start stage */
    _kernel_start_stage = K_SERVICES_STARTUP;

    /* init image loader */
    err = imgload_init();
    if(err != NO_ERROR)
        panic("Failed to init executable image loader with err = %x!\n", err);

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

/* initialization thread */
static int init_thread(void *data)
{
    status_t err;

    /* start init process */
    err = imgload(INIT_SERVICE, PROCESS_ROLE_SERVICE);
    if(err != NO_ERROR)
        panic("Failed to load init %s: %x\n", INIT_SERVICE, err);

    return err;
}
