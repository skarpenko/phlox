/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
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
 * Create user process.
 * Process created in BIRTH state and a pointer to its structure returned
 * on success.
*/
process_t *proc_create_user_process(const char *name, process_t *parent,
    vm_address_space_t *aspace, const char *args, uint role);

/*
 * Destroy process by its id.
*/
status_t proc_destroy_process(proc_id pid);

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

/*
 * Increment references count.
 * Returns pointer to process or NULL if process
 * is not available anymore.
*/
process_t *proc_inc_refcnt(process_t *proc);

/*
 * Returns id of the main thread for process.
 * May return INVALID_THREADID.
 * Process must have reference count incremented.
*/
thread_id proc_get_main_thread_id(process_t *proc);

/*
 * Returns thread structure of the main thread for process.
 * May return NULL.
 * Process must have reference count incremented.
*/
thread_t *proc_get_main_thread(process_t *proc);

/*
 * Returns thread structure of the main thread for process,
 * with lock acquired.
 * May return NULL.
 * Process must have reference count incremented.
 * IMPORTANT NOTE: Should be used with interrupts disabled!
*/
thread_t *proc_get_main_thread_locked(process_t *proc);

/*
 * Returns address space id for the process.
 * May return VM_INVALID_ASPACEID.
 * Process must have reference count incremented.
*/
aspace_id proc_get_aspace_id(process_t *proc);

/*
 * Returns address space for the process.
 * May return NULL.
 * Process must have reference count incremented.
*/
vm_address_space_t *proc_get_aspace(process_t *proc);


#endif
