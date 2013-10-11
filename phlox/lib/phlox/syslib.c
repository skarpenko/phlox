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
