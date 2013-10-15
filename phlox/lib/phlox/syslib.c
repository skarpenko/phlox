/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/

#include <string.h>
#include <phlox/errors.h>
#include <phlox/thread_types.h>
#include <app/syslib.h>
#include <app/syscall.h>



/* print into klog */
int klog_printf(const char *fmt, ...)
{
    int ret;
    va_list args;
    char temp[512];

    /* print into temp string */
    va_start(args, fmt);
    ret = vsnprintf(temp, 512, fmt, args);
    va_end(args);

    if(ret<0)
        return ret;

    /* put string to kernel log */
    if(sys_klog_puts(temp, ret) != NO_ERROR)
        return -1;

    return ret;
}

/* load new system service */
status_t service_load(const char *path)
{
    return sys_svc_load(path, (unsigned)strlen(path), PROCESS_ROLE_SERVICE);
}

/* create new semaphore */
sem_id semaphore_create(const char *name, unsigned max_count, unsigned init_count)
{
    return sys_sem_create(name, name ? (unsigned)strlen(name) : 0, max_count, init_count);
}

/* delete semaphore */
status_t semaphore_delete(sem_id id)
{
    return sys_sem_delete(id);
}

/* count down semaphore */
status_t semaphore_down(sem_id id, unsigned count)
{
    return sys_sem_down(id, count, 0, SYS_SEMF_NOFLAGS);
}

/* try to count down semaphore */
status_t semaphore_try_down(sem_id id, unsigned count)
{
    return sys_sem_down(id, count, 0, SYS_SEMF_TRY);
}

/* count down semaphore with timeout */
status_t semaphore_down_timeout(sem_id id, unsigned count, unsigned timeout_msec)
{
    return sys_sem_down(id, count, timeout_msec, SYS_SEMF_TIMEOUT);
}

/* count up semaphore */
status_t semaphore_up(sem_id id, unsigned count)
{
    return sys_sem_up(id, count);
}

/* get named semaphore */
sem_id semaphore_get_by_name(const char *name)
{
    return sys_sem_get_by_name(name, (unsigned)strlen(name));
}
