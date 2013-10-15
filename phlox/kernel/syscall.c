/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/types.h>
#include <phlox/processor.h>
#include <phlox/kernel.h>
#include <phlox/vm.h>
#include <phlox/errors.h>
#include <phlox/heap.h>
#include <phlox/klog.h>
#include <phlox/imgload.h>
#include <phlox/process.h>
#include <phlox/thread.h>
#include <phlox/sem.h>
#include <phlox/syscall.h>


/* generic syscall function */
typedef void (*syscall_func)(void);

/* syscall table item */
struct syscall_table_entry {
    syscall_func syscall;
};

#define SYSCALL_ENTRY(func) { ((syscall_func)(func)) }


/* No-op syscall */
static status_t syscall_null(void)
{
    return NO_ERROR;
}

/* Not implemented syscall */
static status_t syscall_not_impl(void)
{
    return ERR_SCL_NOT_IMPLEMENTED;
}

/* Put string into kernel log */
static status_t syscall_klog_puts(const char *str, unsigned len)
{
    char *tmp;
    status_t err;

    /* should be less than page */
    if(!str || !len || len > PAGE_SIZE-1)
        return ERR_INVALID_ARGS;

    /* allocate temporary buffer for userspace data */
    tmp = (char *)kmalloc(len+1);
    if(tmp == NULL)
        return ERR_NO_MEMORY;

    /* copy from userspace */
    err = cpy_from_uspace(tmp, str, len);
    if(err != NO_ERROR) {
        kfree(tmp);
        return err;
    }

    /* put string into the log */
    tmp[len] = 0;
    klog_puts(tmp);

    kfree(tmp);

    return NO_ERROR;
}

/* Create user space process from ELF file stored on BootFS */
static status_t syscall_svc_load(const char *path, unsigned len, unsigned role)
{
    char *tmp;
    status_t err;

    /* check path length */
    if(!path || !len || len > PAGE_SIZE-1)
        return ERR_INVALID_ARGS;

    /* allocate temporary buffer for userspace data */
    tmp = (char *)kmalloc(len+1);
    if(tmp == NULL)
        return ERR_NO_MEMORY;

    /* copy path to file from userspace */
    err = cpy_from_uspace(tmp, path, len);
    if(err != NO_ERROR) {
        kfree(tmp);
        return err;
    }

    tmp[len] = 0;

    /* load image */
    err = imgload(tmp, role);

    kfree(tmp);

    return err;
}

/* Create new user thread */
static thread_id syscall_create_thread(int (*func)(void*), void *data, bool suspended,
    unsigned stack_size)
{
    thread_id tid = INVALID_THREADID;
    object_id stack_id;
    process_t *proc;

    /* check that passed addresses are within user space */
    if(!is_user_address((addr_t)func) || (data && !is_user_address((addr_t)data)))
        return tid;

    /* choose stack size  */
    if(stack_size) {
        stack_size = ROUNDUP(stack_size, PAGE_SIZE);
        if(stack_size/PAGE_SIZE > USER_STACK_SIZE_MAX)
            return tid;
    } else
        stack_size = USER_STACK_SIZE*PAGE_SIZE;

    /* create memory object for user space thread stack */
    stack_id = vm_create_object(NULL, stack_size, VM_OBJECT_PROTECT_READ | VM_OBJECT_PROTECT_WRITE);
    if(stack_id == VM_INVALID_OBJECTID)
        return tid;

    /* current user process */
    proc = proc_get_current_process();

    /* create thread */
    tid = thread_create_user_thread(NULL, proc, (addr_t)func, data, stack_id, 0, suspended);
    if(tid == INVALID_THREADID)
        vm_delete_object(stack_id);

    /* return process structure to the system */
    proc_put_process(proc);

    return tid;
}

/* Suspend thread */
static status_t syscall_thread_suspend(thread_id tid)
{
    unsigned long irqs_state;
    status_t err = NO_ERROR;
    thread_t *current = thread_get_current_thread();
    thread_t *thread;

    /* check if it's possible to change state of requested thread */
    local_irqs_save_and_disable(irqs_state);
    thread = thread_get_thread_struct_locked(tid);
    if(thread == NULL)
        err = ERR_MT_INVALID_HANDLE;
    if(thread->process != current->process)
        err = ERR_NO_PERM;
    thread_unlock_thread(thread);
    local_irqs_restore(irqs_state);

    if(err != NO_ERROR)
        return err;

    return thread_suspend(tid);
}

/* Resume thread */
static status_t syscall_thread_resume(thread_id tid)
{
    unsigned long irqs_state;
    status_t err = NO_ERROR;
    thread_t *current = thread_get_current_thread();
    thread_t *thread;

    /* check if it's possible to change state of requested thread */
    local_irqs_save_and_disable(irqs_state);
    thread = thread_get_thread_struct_locked(tid);
    if(thread == NULL)
        err = ERR_MT_INVALID_HANDLE;
    if(thread->process != current->process)
        err = ERR_NO_PERM;
    thread_unlock_thread(thread);
    local_irqs_restore(irqs_state);

    if(err != NO_ERROR)
        return err;

    return thread_resume(tid);
}

/* Pass control to another thread */
static void syscall_thread_yield(void)
{
    thread_yield();
}

/* Put thread into temporary sleep */
static void syscall_thread_sleep(unsigned msec)
{
    thread_sleep(msec);
}

/* Terminate current thread */
static void syscall_thread_exit(int exitcode)
{
    thread_exit(exitcode);
}

/* Return id for current thread */
static thread_id syscall_current_thread_id(void)
{
    return thread_get_current_thread_id();
}

/* create new semaphore */
static sem_id syscall_sem_create(const char *name, unsigned len, unsigned max_count, unsigned init_count)
{
    status_t err;
    sem_id id;
    char *tmp = NULL;

    /* check correctness of name argument if specified */
    if(name && len) {
        /* check name length */
        if(len > PAGE_SIZE-1)
            return ERR_INVALID_ARGS;

        /* allocate temporary buffer for user space data */
        tmp = (char *)kmalloc(len+1);
        if(tmp == NULL)
            return INVALID_SEMID;

        /* copy semaphore name from user space */
        err = cpy_from_uspace(tmp, name, len);
        if(err != NO_ERROR) {
            kfree(tmp);
            return INVALID_SEMID;
        }

        tmp[len] = 0;
    }

    /* try to create semaphore */
    id = sem_create(tmp, max_count, init_count);

    if(tmp)
        kfree(tmp);

    return id;
}

/* delete semaphore */
static status_t syscall_sem_delete(sem_id id)
{
    proc_id owner_id = sem_get_owner(id);

    /* check argument is valid */
    if(owner_id == INVALID_PROCESSID)
        return ERR_INVALID_ARGS;

    /* cannot delete semaphores created by other processes */
    if(owner_id != proc_get_current_process_id())
        return ERR_NO_PERM;

    return sem_delete(id);
}

/* count down semaphore */
static status_t syscall_sem_down(sem_id id, unsigned count, unsigned timeout_msec, flags_t flags)
{
    return sem_down_ex(id, count, timeout_msec, flags);
}

/* count up semaphore */
static status_t syscall_sem_up(sem_id id, unsigned count)
{
    return sem_up(id, count);
}

/* get public semaphore by name */
static sem_id syscall_sem_get_by_name(const char *name, unsigned len)
{
    status_t err;
    sem_id id;
    char *tmp;

    /* check arguments */
    if(!name || !len || len > PAGE_SIZE-1)
        return ERR_INVALID_ARGS;

    /* allocate temporary buffer for user space data */
    tmp = (char *)kmalloc(len+1);
    if(tmp == NULL)
        return ERR_NO_MEMORY;

    /* copy name from user space */
    err = cpy_from_uspace(tmp, name, len);
    if(err != NO_ERROR) {
        kfree(tmp);
        return err;
    }

    tmp[len] = 0;

    /* get semaphore id */
    id = sem_get_by_name(tmp);

    kfree(tmp); /* free name data */

    /* kernel created semaphores are invisible for user space */
    if(id != INVALID_SEMID && proc_get_kernel_process_id() == sem_get_owner(id))
        id = INVALID_SEMID;

    return id;
}

/* allocate virtual memory */
static void* syscall_virtmem_alloc(ulong size)
{
    aspace_id aid;
    object_id oid;
    addr_t vaddr;
    status_t err;

    /* check argument */
    if(!size)
        return NULL;

    /* round size to page boundary */
    size = ROUNDUP(size, PAGE_SIZE);

    /* create virtual memory object */
    oid = vm_create_object(NULL, size, VM_OBJECT_PROTECT_READ | VM_OBJECT_PROTECT_WRITE);
    if(oid == VM_INVALID_OBJECTID)
        return NULL;

    /* current user address space */
    aid = proc_get_aspace_id(proc_get_current_process());

    /* map object into user address space */
    err = vm_map_object(aid, oid, VM_PROT_USER_READ | VM_PROT_USER_WRITE, &vaddr);
    if(err != NO_ERROR)
        return NULL;

    return (void*)vaddr;
}

/* free virtual memory */
static status_t syscall_virtmem_free(void *ptr)
{
    status_t err;
    addr_t vaddr = (addr_t)ptr;
    object_id oid;
    aspace_id aid;

    /* check argument */
    if(!is_user_address(vaddr))
        return ERR_INVALID_ARGS;

    /* current user address space */
    aid = proc_get_aspace_id(proc_get_current_process());

    /* query virtual memory object */
    oid = vm_query_object(aid, vaddr);
    if(oid == VM_INVALID_OBJECTID)
        return ERR_NO_OBJECT;

    /* unmap object */
    err = vm_unmap_object(aid, vaddr);
    if(err != NO_ERROR)
        return err;

    /* mark object for deletion and return */
    return vm_delete_object(oid);
}


/* system calls table */
const struct syscall_table_entry syscall_table[NR_SYSCALLS] = {
/*  0 */    SYSCALL_ENTRY(syscall_null),
/*  1 */    SYSCALL_ENTRY(syscall_klog_puts),
/*  2 */    SYSCALL_ENTRY(syscall_svc_load),
/*  3 */    SYSCALL_ENTRY(syscall_create_thread),
/*  4 */    SYSCALL_ENTRY(syscall_thread_suspend),
/*  5 */    SYSCALL_ENTRY(syscall_thread_resume),
/*  6 */    SYSCALL_ENTRY(syscall_thread_yield),
/*  7 */    SYSCALL_ENTRY(syscall_thread_sleep),
/*  8 */    SYSCALL_ENTRY(syscall_thread_exit),
/*  9 */    SYSCALL_ENTRY(syscall_current_thread_id),
/* 10 */    SYSCALL_ENTRY(syscall_sem_create),
/* 11 */    SYSCALL_ENTRY(syscall_sem_delete),
/* 12 */    SYSCALL_ENTRY(syscall_sem_down),
/* 13 */    SYSCALL_ENTRY(syscall_sem_up),
/* 14 */    SYSCALL_ENTRY(syscall_sem_get_by_name),
/* 15 */    SYSCALL_ENTRY(syscall_virtmem_alloc),
/* 16 */    SYSCALL_ENTRY(syscall_virtmem_free),
};

/* number of entries at system calls table */
const int nr_syscall_table_entries = sizeof(syscall_table)/sizeof(struct syscall_table_entry);
