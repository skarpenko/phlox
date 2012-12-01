/*
* Copyright 2007-2011, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_PROCESS_H_
#define _PHLOX_PROCESS_H_

#include <phlox/types.h>
#include <phlox/ktypes.h>
#include <phlox/kargs.h>
#include <phlox/thread_types.h>
#include <phlox/arch/process.h>


/* Reserved ID */
#define INVALID_PROCESSID  ((proc_id)0)  /* Invalid process ID */


/*
 * Returns kernel process ID to caller
*/
proc_id proc_get_kernel_process_id(void);

/*
 * Returns kernel process struct to caller
*/
process_t *proc_get_kernel_process(void);

/*
 * Returns process structure by its id
*/
process_t *proc_get_process_by_id(proc_id pid);

/*
 * Returns current process id
*/
proc_id proc_get_current_process_id(void);

/*
 * Returns current process structure
*/
process_t *proc_get_current_process(void);

/*
 * Put previously taken process structure
*/
void proc_put_process(process_t *proc);

#endif
