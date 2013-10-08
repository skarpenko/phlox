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



/* NULL system call  */
status_t sys_null(void);

/*
 * Put string into kernel log
 */
status_t sys_klog_puts(const char *str, unsigned len);


#ifdef __cplusplus
}
#endif

#endif
