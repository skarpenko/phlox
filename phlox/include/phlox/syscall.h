/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_SYSCALL_H
#define _PHLOX_SYSCALL_H


/* Phlox kernel system calls */
#define SYSCALL_NULL                         0
#define SYSCALL_KLOG_PUTS                    1
#define SYSCALL_SVC_LOAD                     2
#define SYSCALL_CREATE_THREAD                3
#define SYSCALL_THREAD_SUSPEND               4
#define SYSCALL_THREAD_RESUME                5
#define SYSCALL_THREAD_YIELD                 6
#define SYSCALL_THREAD_SLEEP                 7
#define SYSCALL_THREAD_EXIT                  8
#define SYSCALL_CURRENT_THREAD_ID            9
#define SYSCALL_SEM_CREATE                  10
#define SYSCALL_SEM_DELETE                  11
#define SYSCALL_SEM_DOWN                    12
#define SYSCALL_SEM_UP                      13
#define SYSCALL_SEM_GET_BY_NAME             14
#define SYSCALL_VIRTMEM_ALLOC               15
#define SYSCALL_VIRTMEM_FREE                16

/* Number of system calls */
#define NR_SYSCALLS                         17

/* Reserved system call value */
#define INVALID_SYSCALL                     -1


#endif
