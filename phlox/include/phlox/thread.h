/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
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


/* Check thread state */
#define IS_THREAD_STATE_READY(thread)     (thread->state==THREAD_STATE_READY)
#define IS_THREAD_STATE_BIRTH(thread)     (thread->state==THREAD_STATE_BIRTH)
#define IS_THREAD_STATE_DEATH(thread)     (thread->state==THREAD_STATE_DEATH)
#define IS_THREAD_STATE_RUNNING(thread)   (thread->state==THREAD_STATE_RUNNING)
#define IS_THREAD_STATE_WAITING(thread)   (thread->state==THREAD_STATE_WAITING)
#define IS_THREAD_STATE_SLEEPING(thread)  (thread->state==THREAD_STATE_SLEEPING)
#define IS_THREAD_STATE_SUSPENDED(thread) (thread->state==THREAD_STATE_SUSPENDED)
#define IS_THREAD_STATE_DEAD(thread)      (thread->state==THREAD_STATE_DEAD)

/* Check thread next state */
#define IS_THREAD_NEXSTATE_READY(thread)     (thread->next_state==THREAD_STATE_READY)
#define IS_THREAD_NEXSTATE_BIRTH(thread)     (thread->next_state==THREAD_STATE_BIRTH)
#define IS_THREAD_NEXSTATE_DEATH(thread)     (thread->next_state==THREAD_STATE_DEATH)
#define IS_THREAD_NEXSTATE_RUNNING(thread)   (thread->next_state==THREAD_STATE_RUNNING)
#define IS_THREAD_NEXSTATE_WAITING(thread)   (thread->next_state==THREAD_STATE_WAITING)
#define IS_THREAD_NEXSTATE_SLEEPING(thread)  (thread->next_state==THREAD_STATE_SLEEPING)
#define IS_THREAD_NEXSTATE_SUSPENDED(thread) (thread->next_state==THREAD_STATE_SUSPENDED)
#define IS_THREAD_NEXSTATE_DEAD(thread)      (thread->next_state==THREAD_STATE_DEAD)


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

/*
 * Put current thread into bed for a given count of
 * milliseconds.
*/
void thread_sleep(uint msec);

/*
 * Put thread into bed by its id.
*/
status_t thread_sleep_id(thread_id tid, uint msec);



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


/*** Other utility routines ***/

/*
 * Register thread termination callback.
 *
 * Thread pointed by corresponding cbd field must be locked before call.
 * If callback was registered successfully pointer to cbd returned
 * or NULL if some error occured.
 *
 * Callbacks are called in terminating thread context with no thread lock
 * acquired. It is safe to free thread_cbd_t structure inside of callback
 * being called.
*/
thread_cbd_t *thread_register_term_cb(thread_cbd_t *cbd);

/*
 * Unregister previously registered callback.
 *
 * Thread pointed by corresponding cbd field must be locked before call.
 *
 * cbd structure must be the same as used in register call.
*/
void thread_unregister_term_cb(thread_cbd_t *cbd);

/*
 * Return process data for the thread.
 * May return NULL.
*/
process_t *thread_get_process(thread_t *thread);

/*
 * Return process data for the thread.
 * Assumes thread lock was acquired before call and
 * interrupts was disabled.
 * May return NULL.
*/
process_t *thread_get_process_nolock(thread_t *thread);

/*
 * Return process id for the thread.
 * May return INVALID_PROCESSID.
*/
proc_id thread_get_process_id(thread_t *thread);


#endif
