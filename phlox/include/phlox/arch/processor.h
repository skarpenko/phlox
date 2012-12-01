/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_PROCESSOR_H_
#define _PHLOX_ARCH_PROCESSOR_H_

#include INC_ARCH(phlox/arch,processor.h)

/*** This ones called during processor's init process ***/
/* architecture specific processor module init */
void arch_processor_mod_init(arch_processor_t *bsp);
/* architecture specific processor set init */
void arch_processor_set_init(arch_processor_set_t *aps, kernel_args_t *kargs, uint curr_cpu);
/* architecture specific processor init */
void arch_processor_init(arch_processor_t *ap, kernel_args_t *kargs, uint curr_cpu);

#endif
