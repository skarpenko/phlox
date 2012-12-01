/*
* Copyright 2007-2011, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_PARAM_H_
#define _PHLOX_PARAM_H_

/*
 * Set HZ variable for kernel internal timer frequency
 * Note: Use HZ variable instead of SYSCFG_KERNEL_HZ
 */
#ifndef SYSCFG_KERNEL_HZ
#  error SYSCFG_KERNEL_HZ is not defined in phlox/sysconfig.h!
#else
#  define HZ  SYSCFG_KERNEL_HZ
#endif

/*
 * Max. name length for kernel objects
 */
#define SYS_MAX_OS_NAME_LEN    64

/*
 * Max. process arguments length
 */
#define SYS_MAX_PROC_ARGS_LEN  512

/*
 * Max. semaphores count (must be power of 2)
 */
#define SYS_MAX_SEM_COUNT      8192


#endif
