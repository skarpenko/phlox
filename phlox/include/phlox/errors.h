/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ERRORS_H
#define _PHLOX_ERRORS_H

/* No error */
#define NO_ERROR  0x00000000

/* General errors */
enum PhloxGeneralErrors {
    ERR_GENERAL_BASE = 0x80000000,
    ERR_GENERAL = ERR_GENERAL_BASE,
    ERR_INVALID_ARGS,
    ERR_NO_MEMORY,
    ERR_OUT_OF_RANGE,
    ERR_NAME_NOT_UNIQUE,
    ERR_GENERAL_LIMIT
};

/* Virtual memory errors */
enum PhloxVirtualMemoryErrors {
    ERR_VM_GENERAL_BASE = 0x81000000,
    ERR_VM_GENERAL = ERR_VM_GENERAL_BASE,
    ERR_VM_INVALID_ASPACE,
    ERR_VM_INVALID_OBJECT,
    ERR_VM_BAD_ADDRESS,
    ERR_VM_BAD_OFFSET,
    ERR_VM_NO_MAPPING,
    ERR_VM_NO_MAPPING_GAP,
    ERR_VM_NO_UPAGE,
    ERR_VM_UPAGE_EXISTS,
    ERR_VM_PAGE_NOT_PRESENT,
    ERR_VM_GENERAL_LIMIT
};

/* Multithreading errors */
enum PhloxMultithreadingErrors {
    ERR_MT_GENERAL_BASE = 0x82000000,
    ERR_MT_GENERAL = ERR_MT_GENERAL_BASE,
    ERR_MT_INVALID_HANDLE,
    ERR_MT_INVALID_STATE,
    ERR_MT_GENERAL_LIMIT
};

/* Semaphores errors */
enum PhloxSemaphoresErrors {
    ERR_SEM_GENERAL_BASE = 0x83000000,
    ERR_SEM_GENERAL = ERR_SEM_GENERAL_BASE,
    ERR_SEM_INVALID_HANDLE,
    ERR_SEM_INVALID_VALUE,
    ERR_SEM_DELETED,
    ERR_SEM_TIMEOUT,
    ERR_SEM_TRY_FAILED,
    ERR_SEM_GENERAL_LIMIT
};

/* Mutexes errors */
enum PhloxMutexesErrors {
    ERR_MTX_GENERAL_BASE = 0x83100000,
    ERR_MTX_GENERAL = ERR_MTX_GENERAL_BASE,
    ERR_MTX_INVALID_MUTEX,
    ERR_MTX_SEM_FAILURE,
    ERR_MTX_NOT_AN_OWNER,
    ERR_MTX_TRY_FAILED,
    ERR_MTX_GENERAL_LIMIT
};

/* System calls errors */
enum SystemCallsErrors {
    ERR_SCL_GENERAL_BASE = 0x84000000,
    ERR_SCL_GENERAL = ERR_SCL_GENERAL_BASE,
    ERR_SCL_INVALID,
    ERR_SCL_NOT_IMPLEMENTED,
    ERR_SCL_GENERAL_LIMIT
};

#endif
