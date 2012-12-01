/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_THREAD_H_
#define _PHLOX_THREAD_H_

#include <phlox/types.h>
#include <phlox/ktypes.h>
#include <phlox/kargs.h>
#include <phlox/thread_types.h>
#include <phlox/arch/thread.h>


/* Reserved ID */
#define INVALID_THREADID  ((thread_id)0)  /* Invalid thread ID */


/*
 * Starts threading initialization stages sequence.
 * Called only within system start up.
*/
status_t threading_init(kernel_args_t *kargs, uint curr_cpu);

/*
 * Get current thread information
*/
thread_t *thread_get_current_thread(void);

/*
 * Get current thread id
*/
thread_id thread_get_current_thread_id(void);


#endif
