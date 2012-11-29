/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_PROCESSOR_H_
#define _PHLOX_PROCESSOR_H_

#include <phlox/ktypes.h>
#include <arch/cpu.h>
#include <arch/arch_bits.h>
#include <arch/arch_data.h>
#include <phlox/arch/processor.h>

/* invalidate all entries in Translation Lookaside Buffer of MMU */
#define invalidateTLB()  arch_invalidateTLB()

/* invalidate single entry in Translation Lookaside Buffer */
#define invalidateTLBentry(virt_addr)  arch_invalidateTLBentry(virt_addr)

/* no operation  */
#define nop()  arch_nop()

/* good thing to insert into busy-wait loops */
#define cpu_relax()  arch_cpu_relax()

/* interrupt control */
#define local_irqs_enable()    arch_local_irqs_enable()
#define local_irqs_disable()   arch_local_irqs_disable()
#define local_irqs_disabled()  arch_local_irqs_disabled()

/* used in the idle loop */
#define safe_halt()  arch_safe_halt()

/* used to shutdown the processor */
#define halt()  arch_halt()

#endif
