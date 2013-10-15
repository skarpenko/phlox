/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_SYSCALL_H
#define _PHLOX_SYSCALL_H


/* Phlox kernel system calls */
enum PhloxSyscallNumbers {
    SYSCALL_NULL,
    SYSCALL_KLOG_PUTS,
    SYSCALL_SVC_LOAD,
    SYSCALL_CREATE_THREAD,
    SYSCALL_THREAD_SUSPEND,
    SYSCALL_THREAD_RESUME,
    SYSCALL_THREAD_YIELD,
    SYSCALL_THREAD_SLEEP,
    SYSCALL_THREAD_EXIT,
    SYSCALL_CURRENT_THREAD_ID,
    SYSCALL_SEM_CREATE,
    SYSCALL_SEM_DELETE,
    SYSCALL_SEM_DOWN,
    SYSCALL_SEM_UP,
    SYSCALL_SEM_GET_BY_NAME,
    SYSCALL_VIRTMEM_ALLOC,
    SYSCALL_VIRTMEM_FREE,
    /* Number of system calls */
    NR_SYSCALLS
};


#endif
