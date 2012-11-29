/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/types.h>
#include <phlox/kernel.h>
#include <phlox/kargs.h>
#include <arch/arch_data.h>
#include <arch/arch_bits.h>
#include <phlox/errors.h>
#include <phlox/arch/vm_translation_map.h>
#include <phlox/vm_page.h>
#include <phlox/vm.h>

/* platform-specific init for virtual memory manager */
status_t platform_vm_init(kernel_args_t *kargs)
{
    return NO_ERROR;
}

/* final stage of platform-specific init */
status_t platform_vm_init_final(kernel_args_t *kargs)
{
    return NO_ERROR;
}
