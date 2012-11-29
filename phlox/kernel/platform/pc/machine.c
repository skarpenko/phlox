/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/kernel.h>
#include <phlox/machine.h>
#include <phlox/platform/pc/pic.h>
#include <phlox/platform/pc/pit.h>

/* PC-specific machine init */
uint32 platform_machine_init(kernel_args_t *kargs)
{
    /* init Programmable Interrupt Controllers */
    pic_init();
    /* init Programmable Interval Timer */
    pit_init();

    return 0; /* return success */
}
