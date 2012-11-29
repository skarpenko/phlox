/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_SYSCONFIG_H
#define _PHLOX_SYSCONFIG_H

/*** System configuration, edit these to configure the build ***/

/* Compile in support for SMP or not (not supported for now) */
#define SYSCFG_SMP_SUPPORT 0

/* Maximum number of supported cpus */
#define SYSCFG_MAX_CPUS 32

#if !SYSCFG_SMP_SUPPORT
#undef  SYSCFG_MAX_CPUS
#define SYSCFG_MAX_CPUS 1
#endif

#endif
