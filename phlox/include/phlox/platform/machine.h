/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_PLATFORM_MACHINE_H_
#define _PHLOX_PLATFORM_MACHINE_H_

#include INC_PLATF(phlox/platform,machine.h)

/*
 * Platform specific machine initialization routine.
 * Called during system boot up.
*/
uint32 platform_machine_init(kernel_args_t *kargs);

#endif
