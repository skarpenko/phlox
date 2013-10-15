/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/

#ifndef _APP_SYSLIB_H_
#define _APP_SYSLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <sys/types.h>
#include <stdarg.h>
#include <app/syscall.h>


/* standard string formatting routines */
int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
int vscnprintf(char *buf, size_t size, const char *fmt, va_list args);
int snprintf(char *buf, size_t size, const char *fmt, ...);
int scnprintf(char *buf, size_t size, const char *fmt, ...);
int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);

/* print into kernel log  */
int klog_printf(const char *fmt, ...);

/*
 * Load new system service from BootFS image
 *
 * Arguments:
 *    path - path to executable.
*/
status_t service_load(const char *path);

/*
 * Suspend execution of current thread
*/
static inline status_t thread_suspend_current(void)
{
    return sys_thread_suspend(sys_current_thread_id());
}

/*
 * Create semaphore
 *
 * Arguments:
 *   name       - semaphore name (NULL for private semaphores);
 *   max_count  - maximum count for semaphore;
 *   init_count - initial count.
*/
sem_id semaphore_create(const char *name, unsigned max_count, unsigned init_count);

/*
 * Delete semaphore
 *
 * Arguments:
 *   id  - semaphore id.
*/
status_t semaphore_delete(sem_id id);

/*
 * Semaphore count down
 *
 * Arguments:
 *   id     - semaphore id;
 *   count  - value to count down.
*/
status_t semaphore_down(sem_id id, unsigned count);

/*
 * Try to count down semaphore
 *
 * Arguments:
 *   id     - semaphore id;
 *   count  - value to count down.
*/
status_t semaphore_try_down(sem_id id, unsigned count);

/*
 * Semaphore count down with timeout
 *
 * Arguments:
 *   id            - semaphore id;
 *   count         - value to count down;
 *   timeout_msec  - timeout in milliseconds.
*/
status_t semaphore_down_timeout(sem_id id, unsigned count, unsigned timeout_msec);

/*
 * Semaphore count up
 *
 * Arguments:
 *   id     - semaphore id;
 *   count  - value to count up.
*/
status_t semaphore_up(sem_id id, unsigned count);

/*
 * Get public semaphore by its name
 *
 * Arguments:
 *   name  - semaphore name.
*/
sem_id semaphore_get_by_name(const char *name);


#ifdef __cplusplus
}
#endif

#endif
