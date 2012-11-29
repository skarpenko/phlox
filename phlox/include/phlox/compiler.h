/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_COMPILER_H
#define _PHLOX_COMPILER_H

#ifndef __ASSEMBLY__

#if __GNUC__ == 3
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#define _PACKED __attribute__((packed))
#define _ALIGNED(x) __attribute__((aligned(x)))

/* Optimization barrier */
/* The "volatile" is due to gcc bugs */
#define barrier() __asm__ __volatile__("": : :"memory")

#endif /* #ifndef __ASSEMBLY__ */

#endif
