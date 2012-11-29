/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_MACHINE_H_
#define _PHLOX_MACHINE_H_

#include <phlox/ktypes.h>
#include <phlox/kargs.h>
#include <phlox/platform/machine.h>

/*
 * Machine initialization routine. Performs initialization of
 * machine-specific hardware and kernel interfaces.
 * Called during system boot up.
*/
uint32 machine_init(kernel_args_t *kargs);

#endif
