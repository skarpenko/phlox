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
#include <phlox/vm_page.h>
#include <phlox/vm.h>

/* architecture-specific init for virtual memory manager */
void arch_init_vm(kernel_args_t *kargs) {
    /* do something */
    uint32 i, n=PAGE_SIZE*PAGE_SIZE;
    uint8 *buf;

    /* translation map module init */
    arch_vm_transmap_init(kargs);

    kprint("start vm_alloc_from_kargs test....(%d bytes to allocate)\n", n);
    buf = (uint8 *)vm_alloc_from_kargs(kargs, n, VM_LOCK_KERNEL | VM_LOCK_RW);
    kprint("touching allocated memory...\n");
    for(i=0; i<n; i++) buf[i] = 0;
    kprint("tests passed...\n");
}
