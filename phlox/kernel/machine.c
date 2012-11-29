/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/kernel.h>
#include <phlox/machine.h>

/* machine init */
uint32 machine_init(kernel_args_t *kargs) {
    uint32 err;

    /* platform specific machine init */
    err = platform_machine_init(kargs);
    if(err)
       panic("platform_machine_init: failed!\n");

    return 0; /* return successful result */
}
