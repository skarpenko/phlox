/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _I386_TYPES_H
#define _I386_TYPES_H

/* Macroses for cpu-specific includes */
#if CPU_i386
#define INC_CPU(path, x) <path/i386/x>
#endif
#if CPU_i486
#define INC_CPU(path, x) <path/i486/x>
#endif
#if CPU_i586
#define INC_CPU(path, x) <path/i586/x>
#endif
#if CPU_i686
#define INC_CPU(path, x) <path/i686/x>
#endif
#if CPU_Pentium
#define INC_CPU(path, x) <path/pentium/x>
#endif
#if CPU_PentiumMMX
#define INC_CPU(path, x) <path/pentium-mmx/x>
#endif
#if CPU_PentiumPro
#define INC_CPU(path, x) <path/pentium-pro/x>
#endif
#if CPU_Pentium2
#define INC_CPU(path, x) <path/pentium2/x>
#endif
#if CPU_Pentium3
#define INC_CPU(path, x) <path/pentium3/x>
#endif
#if CPU_Pentium4
#define INC_CPU(path, x) <path/pentium4/x>
#endif
#if CPU_K6
#define INC_CPU(path, x) <path/k6/x>
#endif
#if CPU_K6_2
#define INC_CPU(path, x) <path/k6-2/x>
#endif
#if CPU_K6_3
#define INC_CPU(path, x) <path/k6-3/x>
#endif
#if CPU_K7
#define INC_CPU(path, x) <path/k7/x>
#endif
#if CPU_Athlon
#define INC_CPU(path, x) <path/athlon/x>
#endif
#if CPU_Athlon_Tbird
#define INC_CPU(path, x) <path/athlon-tbird/x>
#endif
#if CPU_Athlon4
#define INC_CPU(path, x) <path/athlon4/x>
#endif
#if CPU_AthlonXP
#define INC_CPU(path, x) <path/athlon-xp/x>
#endif
#if CPU_AthlonMP
#define INC_CPU(path, x) <path/athlon-mp/x>
#endif


#ifndef WIN32
typedef volatile unsigned long long vuint64;
typedef unsigned long long           uint64;
typedef volatile long long           vint64;
typedef long long                     int64;
#else
typedef volatile unsigned __int64   vuint64;
typedef unsigned __int64             uint64;
typedef volatile __int64             vint64;
#endif
typedef volatile unsigned int       vuint32;
typedef unsigned int                 uint32;
typedef volatile int                 vint32;
typedef int                           int32;
typedef volatile unsigned short     vuint16;
typedef unsigned short               uint16;
typedef volatile short               vint16;
typedef short                         int16;
typedef volatile unsigned char       vuint8;
typedef unsigned char                 uint8;
typedef volatile char                 vint8;
typedef char                           int8;

#endif
