/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
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

#endif
