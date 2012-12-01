/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <stdarg.h>
#include <arch/cpu.h>
#include <phlox/types.h>
#include <phlox/kernel.h>
#include <phlox/processor.h>
#include <phlox/interrupt.h>
#include <phlox/machine.h>
#include <phlox/kargs.h>
#include <phlox/vm.h>
#include <phlox/klog.h>
#include <phlox/debug.h>
#include <boot/bootfs.h>


/* Global kernel args */
kernel_args_t globalKargs;


void _phlox_kernel_entry(kernel_args_t *kargs, uint32 num_cpu);  /* keep compiler happy */
void _phlox_kernel_entry(kernel_args_t *kargs, uint32 num_cpu)
{
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
      kprint("\nWelcome to Phlox Kernel!\n");
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

       /* init virtual memory manager. */
       vm_init(&globalKargs);
    } else {
       /* wait until BSP completes? */
    }

    panic("kernel test complete. :)\n");
}
