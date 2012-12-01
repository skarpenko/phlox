/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Portions copyright 2001, Travis Geiselbrecht.
* Distributed under the terms of the PhloxOS License.
*/

#ifndef _PHLOX_TYPES_H
#define _PHLOX_TYPES_H



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

typedef long long int64;
typedef unsigned long long uint64;
typedef int int32;
typedef unsigned int uint32;

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
typedef int64 bigtime_t;
typedef uint64 vnode_id;
typedef uint object_id;     /* vm object id     */
typedef uint aspace_id;     /* address space id */
typedef int thread_id;      /* thread id        */
typedef int proc_id;        /* process id       */
typedef int pgrp_id;        /* process group id */
typedef int sess_id;        /* session id       */
typedef int sem_id;         /* semaphore id     */
typedef int port_id;        /* ipc port id      */
typedef int image_id;       /* binary image id  */

# include <stddef.h>

#endif
