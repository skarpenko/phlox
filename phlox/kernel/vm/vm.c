/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/vm_page.h>
#include <phlox/vm.h>
#include <phlox/heap.h>

/* Global variable with Virtual Memory State */
vm_stat_t VM_State;


/* init virtual memory */
uint32 vm_init(kernel_args_t *kargs) {
   uint32 err;
   
   /* execute architecture-specific init */
   err = arch_vm_init(kargs);
   if(err)
      panic("arch_vm_init: failed!\n");

   /* init memory size */
   err = vm_page_preinit(kargs);
   if(err)
      panic("vm_page_preinit: failed!\n");

   /* init kernel's heap */
   {
       addr_t heap_base;
       size_t heap_size;

       /* compute heap size */
       heap_size = ROUNDUP(
                       vm_phys_mem_size() / SYSCFG_KERNEL_HEAP_FRAC,
                       1*1024*1024 /* Mbyte */
                   );
       if(heap_size > SYSCFG_KERNEL_HEAP_MAX)
             heap_size = SYSCFG_KERNEL_HEAP_MAX;

       /* allocate heap area */
       heap_base = vm_alloc_from_kargs(kargs, heap_size, VM_LOCK_KERNEL|VM_LOCK_RW);
       /* init heap */
       heap_init(heap_base, heap_size);
       /* Fuf... Now kmalloc and kfree is available */
   }

   /* init vm page module */
   err = vm_page_init(kargs);
   if(err)
      panic("vm_page_init: failed!\n");

/*** Important note: After this point vm_alloc_from_kargs must not be used
 *** because physical pages bookkeping is turned on.
 ***/

   return 0;
}

/* return available physical memory size */
size_t vm_phys_mem_size(void) {
    return VM_State.total_physical_pages * VM_State.physical_page_size;
}
