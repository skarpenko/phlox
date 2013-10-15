/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/

#include <app/syscall.h>
#include <phlox/syscall.h>


/* null syscall */
status_t sys_null(void)
{
    return __syscall0(SYSCALL_NULL);
}

/* put string into kernel log */
status_t sys_klog_puts(const char *str, unsigned len)
{
    return __syscall2(SYSCALL_KLOG_PUTS, (ulong)str, (ulong)len);
}

/* load new service from BootFS */
status_t sys_svc_load(const char *path, unsigned len, unsigned role)
{
    return __syscall3(SYSCALL_SVC_LOAD, (ulong)path, (ulong)len, (ulong)role);
}

/* create thread */
thread_id sys_create_thread(int (*func)(void*), void *data, bool suspended,
    unsigned stack_size)
{
    return __syscall4(SYSCALL_CREATE_THREAD, (ulong)func, (ulong)data, (ulong)suspended,
        (ulong)stack_size);
}

/* suspend thread */
status_t sys_thread_suspend(thread_id tid)
{
    return __syscall1(SYSCALL_THREAD_SUSPEND, (ulong)tid);
}

/* resume thread */
status_t sys_thread_resume(thread_id tid)
{
    return __syscall1(SYSCALL_THREAD_RESUME, (ulong)tid);
}

/* pass control to another thread */
void sys_thread_yield(void)
{
    __syscall0(SYSCALL_THREAD_YIELD);
}

/* put thread into temporary sleep */
void sys_thread_sleep(unsigned msec)
{
    __syscall1(SYSCALL_THREAD_SLEEP, (ulong)msec);
}

/* terminate current thread */
void sys_thread_exit(int exitcode)
{
    __syscall1(SYSCALL_THREAD_EXIT, (ulong)exitcode);
}

/* return id for current thread */
thread_id sys_current_thread_id(void)
{
    return __syscall0(SYSCALL_CURRENT_THREAD_ID);
}

/* create semaphore */
sem_id sys_sem_create(const char *name, unsigned len, unsigned max_count, unsigned init_count)
{
    return __syscall4(SYSCALL_SEM_CREATE, (ulong)name, (ulong)len, (ulong)max_count,
            (ulong)init_count);
}

/* delete semaphore */
status_t sys_sem_delete(sem_id id)
{
    return __syscall1(SYSCALL_SEM_DELETE, (ulong)id);
}

/* semaphore count down */
status_t sys_sem_down(sem_id id, unsigned count, unsigned timeout_msec, flags_t flags)
{
    return __syscall4(SYSCALL_SEM_DOWN, (ulong)id, (ulong)count, (ulong)timeout_msec,
            (ulong)flags);
}

/* semaphore count up */
status_t sys_sem_up(sem_id id, unsigned count)
{
    return __syscall2(SYSCALL_SEM_UP, (ulong)id, (ulong)count);
}

/* get public semaphore by its name */
sem_id sys_sem_get_by_name(const char *name, unsigned len)
{
    return __syscall2(SYSCALL_SEM_GET_BY_NAME, (ulong)name, (ulong)len);
}

/* allocate virtual memory */
void* sys_virtmem_alloc(ulong size)
{
    return (void*)__syscall1(SYSCALL_VIRTMEM_ALLOC, (ulong)size);
}

/* free virtual memory */
status_t sys_virtmem_free(void *ptr)
{
    return __syscall1(SYSCALL_VIRTMEM_FREE, (ulong)ptr);
}
