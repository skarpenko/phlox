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
