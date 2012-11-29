/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <arch/cpu.h>
#include <arch/arch_bits.h>
#include <arch/arch_data.h>
#include <phlox/interrupt.h>


cpu_gate_desc *idt = NULL;


/* init interrupt handling */
uint32 arch_interrupt_init(kernel_args_t *kargs)
{

    return 0;
}
