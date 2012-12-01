/*
* Copyright 2007-2011, Stepan V.Karpenko. All rights reserved.
* Portions copyright 2001, Travis Geiselbrecht.
* Distributed under the terms of the PhloxOS License.
*/

#ifndef _PHLOX_TYPES_H
#define _PHLOX_TYPES_H

/* Macroses for architecture specific includes */
#if ARCH_i386
#define INC_ARCH(path, x) <path/i386/x>
#endif
#if ARCH_ppc
#define INC_ARCH(path, x) <path/ppc/x>
#endif
#if ARCH_x86_64
#define INC_ARCH(path, x) <path/x86_64/x>
#endif
#if ARCH_arm
#define INC_ARCH(path, x) <path/arm/x>
#endif

/* Macroses for platform specific includes */
#if PLATFORM_pc
#define INC_PLATF(path, x) <path/pc/x>
#endif

/*
 * Check at compile time that something is of a particular type.
 * Always evaluates to 1 so you may use it easily in comparisons.
 */
#define typecheck(type,x) \
({  type __dummy; \
    typeof(x) __dummy2; \
    (void)(&__dummy == &__dummy2); \
    1; \
})

/* Include architecture specific types */
#include INC_ARCH(arch,types.h)

#ifndef __cplusplus

typedef int bool;
#define false ((bool)0)
#define true  ((bool)(-1))

#endif

/*
   XXX serious hack that doesn't really solve the problem.
   As of right now, some versions of the toolchain expect size_t to
   be unsigned long (newer ones than 2.95.2), and the older
   ones need it to be unsigned int. It's an actual failure when
   operator new is declared. This will have to be resolved in the
   future.
*/

#ifdef ARCH_m68k
#define __SIZE_T_LONG       1
#endif

#ifdef ARCH_ppc
#define __SIZE_T_INT        1
#endif

#ifdef ARCH_sh4
#define __SIZE_T_INT        1
#endif

#if !__SIZE_T_LONG && !__SIZE_T_INT
/* when in doubt, set it to LONG */
#define __SIZE_T_LONG       1
#endif

/* uncomment the following if you want to override LONG vs INT */
#if 0
#ifdef __SIZE_T_LONG
#undef __SIZE_T_LONG
#endif
#ifdef __SIZE_T_INT
#undef __SIZE_T_INT
#endif

/* to override the LONG vs INT setting, set *one* of the below to 1 */
#define __SIZE_T_LONG       0
#define __SIZE_T_INT        1
#endif

/* note that ssize_t is not C++ standard, so it belongs here as well
   as anywhere.
*/

#if __SIZE_T_LONG
typedef unsigned long       _phlox_size_t;
typedef signed long         ssize_t;
#elif __SIZE_T_INT
typedef unsigned int        _phlox_size_t;
typedef signed int          ssize_t;
#else
#error "Don't know what size_t should be (int or long)!"
#endif

typedef _phlox_size_t            size_t;

typedef int64                    off_t;

typedef unsigned char            u_char;
typedef unsigned short           u_short;
typedef unsigned int             u_int;
typedef unsigned long            u_long;

typedef unsigned char            uchar;
typedef unsigned short           ushort;
typedef unsigned int             uint;
typedef unsigned long            ulong;

typedef volatile char            vchar;
typedef volatile short           vshort;
typedef volatile int             vint;
typedef volatile long            vlong;

typedef volatile unsigned char   vuchar;
typedef volatile unsigned short  vushort;
typedef volatile unsigned int    vuint;
typedef volatile unsigned long   vulong;

typedef unsigned char            uchar_t;
typedef unsigned short           ushort_t;
typedef unsigned int             uint_t;
typedef unsigned long            ulong_t;

typedef volatile char            vchar_t;
typedef volatile short           vshort_t;
typedef volatile int             vint_t;
typedef volatile long            vlong_t;

typedef volatile unsigned char   vuchar_t;
typedef volatile unsigned short  vushort_t;
typedef volatile unsigned int    vuint_t;
typedef volatile unsigned long   vulong_t;

typedef int                      intptr_t;
typedef unsigned int             uintptr_t;
typedef intptr_t                 ptrdiff_t;


/* system types */
typedef uint32 result_t;
typedef uint32 status_t;
typedef uint32 flags_t;
typedef uint32 time_t;
typedef int64 bigtime_t;
typedef uint64 vnode_id;
typedef uint object_id;     /* vm object id     */
typedef uint aspace_id;     /* address space id */
typedef uint thread_id;     /* thread id        */
typedef uint proc_id;       /* process id       */
typedef uint pgrp_id;       /* process group id */
typedef uint sess_id;       /* session id       */
typedef uint sem_id;        /* semaphore id     */
typedef uint port_id;       /* ipc port id      */
typedef uint image_id;      /* binary image id  */
typedef uint timeout_id;    /* timeout call id  */

# include <stddef.h>

#endif
