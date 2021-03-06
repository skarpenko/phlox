/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_I386_KTYPES_H_
#define _PHLOX_ARCH_I386_KTYPES_H_

typedef uint32 addr_t;
typedef uint32 offs_t;

typedef struct {
    addr_t  start;
    uint32  size;
} addr_range_t;

#endif
