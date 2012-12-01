/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_SYSCONFIG_H
#define _PHLOX_SYSCONFIG_H

/*** System configuration, edit these to configure the build ***/

/* Defines kernel's heap size as RAM_Size / SYSCFG_KERNEL_HEAP_FRAC */
#define SYSCFG_KERNEL_HEAP_FRAC 32

/* Defines maximum kernel's heap size */
#define SYSCFG_KERNEL_HEAP_MAX (16*1024*1024) /* 16Mbytes */

/* Defines internal kernel timer frequency */
#define SYSCFG_KERNEL_HZ  100 /* Hz */

/* Defines kernel log config */
#define SYSCFG_KLOG_NROWS  128  /* Rows number */
#define SYSCFG_KLOG_NCOLS   64  /* Cols number */

/* Compile in support for SMP or not (not supported for now) */
#define SYSCFG_SMP_SUPPORT 0

/* Maximum number of supported cpus */
#define SYSCFG_MAX_CPUS 32

#if !SYSCFG_SMP_SUPPORT
#undef  SYSCFG_MAX_CPUS
#define SYSCFG_MAX_CPUS 1
#endif

#endif
