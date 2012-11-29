/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/vm.h>

/* init virtual memory */
void init_vm(kernel_args_t *kargs) {
   /* execute architecture-specific init */
   arch_init_vm(kargs);
}
