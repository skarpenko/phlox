/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/kernel.h>
#include <phlox/ktypes.h>
#include <phlox/errors.h>
#include <phlox/heap.h>
#include <phlox/klog.h>
#include <phlox/imgload.h>
#include <phlox/process.h>
#include <phlox/thread.h>
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
    if(len > PAGE_SIZE-1)
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
    if(len > 512)
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


/* system calls table */
const struct syscall_table_entry syscall_table[NR_SYSCALLS] = {
/*  0 */    SYSCALL_ENTRY(syscall_null),
/*  1 */    SYSCALL_ENTRY(syscall_klog_puts),
/*  2 */    SYSCALL_ENTRY(syscall_svc_load),
};

/* number of entries at system calls table */
const int nr_syscall_table_entries = sizeof(syscall_table)/sizeof(struct syscall_table_entry);
