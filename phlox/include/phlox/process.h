/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_PROCESS_H_
#define _PHLOX_PROCESS_H_

#include <phlox/ktypes.h>
#include <phlox/arch/process.h>
#include <phlox/thread_types.h>

/* Reserved ID */
#define INVALID_PROCESSID  ((proc_id)0)  /* Invalid process ID */


/*
 * Returns kernel process ID to caller
*/
proc_id proc_get_kernel_proc_id(void);

/*
 * Returns kernel process struct to caller
*/
process_t *proc_get_kernel_proc(void);

#endif
