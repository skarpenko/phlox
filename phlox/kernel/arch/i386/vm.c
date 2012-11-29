/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/types.h>
#include <phlox/kernel.h>
#include <phlox/kargs.h>
#include <arch/arch_data.h>
#include <arch/arch_bits.h>
#include <phlox/arch/vm_transmap.h>
#include <phlox/vm.h>

/* init virtual memory */
void arch_init_vm(kernel_args_t *kargs) {
  vm_transmap_quick_map(kargs, 0, 0, 0);
}

