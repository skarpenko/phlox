/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/

#ifndef _APP_SYSCALL_H_
#define _APP_SYSCALL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>


/* System call with zero arguments */
ulong __syscall0(ulong sys_no);

/* System call with one argument */
ulong __syscall1(ulong sys_no, ulong arg0);

/* System call with two arguments */
ulong __syscall2(ulong sys_no, ulong arg0, ulong arg1);

/* System call with three arguments */
ulong __syscall3(ulong sys_no, ulong arg0, ulong arg1, ulong arg2);

/* System call with four arguments */
ulong __syscall4(ulong sys_no, ulong arg0, ulong arg1, ulong arg2, ulong arg3);


/* NOTE: redefinition of values from kernel headers */
#ifndef INVALID_THREADID
#  define INVALID_THREADID  ((thread_id)0) /* Invalid thread ID */
#endif
#ifndef INVALID_SEMID
#  define INVALID_SEMID     ((sem_id)0)    /* Invalid semaphore ID */
#endif
#ifndef INVALID_PROCESSID
#  define INVALID_PROCESSID ((proc_id)0)   /* Invalid process ID */
#endif


/* NULL system call  */
status_t sys_null(void);

/*
 * Put string into kernel log
 */
status_t sys_klog_puts(const char *str, unsigned len);

/*
 * Load new system service from BootFS
 *
 * Arguments:
 *   path  - path to executable;
 *   len   - path length;
 *   role  - role of the new service.
*/
status_t sys_svc_load(const char *path, unsigned len, unsigned role);

/*
 * Create new thread of execution
 *
 * Arguments:
 *   func        - thread function;
 *   data        - user data;
 *   suspended   - create in suspended state if true;
 *   stack_size  - stack size (0 for default).
 *
 * Return: thread id.
*/
thread_id sys_create_thread(int (*func)(void*), void *data, bool suspended,
    unsigned stack_size);

/*
 * Suspend thread
 *
 * Arguments:
 *   tid - thread id.
*/
status_t sys_thread_suspend(thread_id tid);

/*
 * Resume thread
 *
 * Arguments:
 *   tid - thread id.
*/
status_t sys_thread_resume(thread_id tid);

/* Pass control to another thread */
void sys_thread_yield(void);

/*
 * Put thread to sleep
 *
 * Arguments:
 *   msec - milliseconds to sleep.
*/
void sys_thread_sleep(unsigned msec);

/*
 * Terminate thread
 *
 * Arguments:
 *   exitcode  - thread exit code.
*/
void sys_thread_exit(int exitcode);

/* Get current thread id */
thread_id sys_current_thread_id(void);

/*
 * Create semaphore
 *
 * Arguments:
 *   name       - semaphore name (NULL for private semaphores);
 *   len        - semaphore name length;
 *   max_count  - maximum count for semaphore;
 *   init_count - initial count.
*/
sem_id sys_sem_create(const char *name, unsigned len, unsigned max_count, unsigned init_count);

/*
 * Delete semaphore
 *
 * Arguments:
 *   id - semaphore id.
*/
status_t sys_sem_delete(sem_id id);

/*
 * Semaphore count down
 *
 * Arguments:
 *   id            - semaphore id;
 *   count         - value to count down;
 *   timeout_msec  - timeout if specified in flags;
 *   flags         - call flags.
*/
status_t sys_sem_down(sem_id id, unsigned count, unsigned timeout_msec, flags_t flags);

/* Flags to use in sys_sem_down_ex() routine */
enum {
    SYS_SEMF_NOFLAGS = 0x0,  /* No flags */
    SYS_SEMF_TIMEOUT = 0x1,  /* Use timeout param */
    SYS_SEMF_TRY     = 0x2   /* Try to count down */
};

/*
 * Semaphore count up
 *
 * Arguments:
 *   id     - semaphore id;
 *   count  - value to count up.
*/
status_t sys_sem_up(sem_id id, unsigned count);

/*
 * Get public semaphore by its name
 *
 * Arguments:
 *   name  - semaphore name;
 *   len   - semaphore name length.
*/
sem_id sys_sem_get_by_name(const char *name, unsigned len);

/*
 * Allocate virtual memory
 *
 * Arguments:
 *   size - size of memory block to allocate.
*/
void* sys_virtmem_alloc(ulong size);

/*
 * Free virtual memory
 *
 * Arguments:
 *   ptr - pointer to previously allocated region.
*/
status_t sys_virtmem_free(void *ptr);


#ifdef __cplusplus
}
#endif

#endif
