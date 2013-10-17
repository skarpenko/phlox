/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_PROCESS_H_
#define _PHLOX_ARCH_PROCESS_H_

#include <phlox/types.h>
#include <phlox/ktypes.h>
#include <phlox/kargs.h>
#include INC_ARCH(phlox/arch,process.h)


/*
 * Architecture-specific process module init.
 * Called during threading initialization!
*/
status_t arch_process_init(kernel_args_t *kargs);

/*
 * Architecture-specific process module per CPU init.
 * Called during threading initialization!
*/
status_t arch_process_init_per_cpu(kernel_args_t *kargs, uint curr_cpu);

/*
 * Called during creating process for arch-specific initialization.
 */
status_t arch_init_process_struct(process_t *proc);


#endif
