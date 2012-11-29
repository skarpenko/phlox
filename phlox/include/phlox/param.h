/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_PARAM_H_
#define _PHLOX_PARAM_H_

/*
 * Set HZ variable for kernel internal timer frequency
 * Note: Use HZ variable instead of SYSCFG_KERNEL_HZ
 */
#ifndef SYSCFG_KERNEL_HZ
  #warning SYSCFG_KERNEL_HZ is not defined in phlox/sysconfig.h setting HZ to default
  #define HZ  100 /* Hz */
#else
  #define HZ  SYSCFG_KERNEL_HZ
#endif

#endif
