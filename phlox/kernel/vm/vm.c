/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/vm_page.h>
#include <phlox/vm.h>

/* init virtual memory */
uint32 vm_init(kernel_args_t *kargs) {
   /* execute architecture-specific init */
   arch_vm_init(kargs);

   /* init vm page module */
   vm_page_init(kargs);

   return 0;
}
