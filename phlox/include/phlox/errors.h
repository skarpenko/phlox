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
    ERR_GENERAL_LIMIT
};

/* Virtual memory errors */
enum PhloxVirtualMemoryErrors {
    ERR_VM_GENERAL_BASE = 0x81000000,
    ERR_VM_GENERAL = ERR_VM_GENERAL_BASE,
    ERR_VM_GENERAL_LIMIT
};

#endif
