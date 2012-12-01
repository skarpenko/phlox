/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
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

void print_kernel_memory_map(void); /* for DEBUG only */

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


    /* enable interrupts */
    local_irqs_enable();

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

    panic("kernel test complete. :)\n");
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
