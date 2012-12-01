/*
* Copyright 2007-2010, Stepan V.Karpenko. All rights reserved.
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
 * Get structure for specified thread by id.
 * Returns NULL if no such thread.
*/
thread_t *thread_get_thread_struct(thread_id tid);

/*
 * Get current thread information
*/
thread_t *thread_get_current_thread(void);

/*
 * Get current thread id
*/
thread_id thread_get_current_thread_id(void);

/*
 * Returns TRUE if current thread is a kernel thread
*/
bool thread_is_kernel_thread(void);

/*
 * Create new kernel-side thread of execution.
 * Returns new thread id or INVALID_THREADID on error.
 *
 * Params:
 *  name - thread name; func - thread routine; data - optional data;
 *  suspended - if =true thread created initially in suspended state.
*/
thread_id thread_create_kernel_thread(const char *name, int (*func)(void *data),
    void *data, bool suspended);

/*
 * Transfer control to another thread
*/
void thread_yield(void);

/*
 * Terminate current thread with specified exit code
*/
void thread_exit(int exitcode);

/*
 * Suspend currently running thread
*/
status_t thread_suspend_current(void);

/*
 * Suspend specified thread
*/
status_t thread_suspend(thread_id tid);

/*
 * Resume specified thread
*/
status_t thread_resume(thread_id tid);



/*** Routines for short-term thread locking ****/

/*
 * IMPORTANT NOTE!
 *
 * Use thread locks with care. In a sequence of multiple lock operations
 * thread lock must be acquired first to avoid possible deadlocks.
 * Never acquire multiple thread locks with thread_lock_thread()!
 *
 * Scheduler may ignore locked thread or wait until lock will be released.
 * So... It's _strongly_ recommended to acquire thread locks with interrupts
 * disabled and for very short periods of time.
*/

/*
 * Try to acquire thread lock
*/
static inline bool thread_trylock_thread(thread_t *thread)
{
    return (bool)spin_trylock(&thread->lock);
}

/*
 * Lock thread or wait until it will be unlocked
*/
static inline void thread_lock_thread(thread_t *thread)
{
    spin_lock(&thread->lock);
}

/*
 * Unlock thread
*/
static inline void thread_unlock_thread(thread_t *thread)
{
    spin_unlock(&thread->lock);
}

/*
 * Returns thread structure for specified thread with lock acquired
*/
thread_t *thread_get_thread_struct_locked(thread_id tid);

/*
 * Returns current thread with lock acquired
*/
thread_t *thread_get_current_thread_locked(void);


#endif
