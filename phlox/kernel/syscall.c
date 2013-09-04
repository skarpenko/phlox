/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/kernel.h>
#include <phlox/ktypes.h>
#include <phlox/errors.h>
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

/* system calls table */
const struct syscall_table_entry syscall_table[NR_SYSCALLS] = {
/*  0 */    SYSCALL_ENTRY(syscall_null),
/*  1 */    SYSCALL_ENTRY(syscall_not_impl),
/*  2 */    SYSCALL_ENTRY(syscall_not_impl),
};

/* number of entries at system calls table */
const int nr_syscall_table_entries = sizeof(syscall_table)/sizeof(struct syscall_table_entry);
