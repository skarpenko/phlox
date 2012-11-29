/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_I386_PROCESSOR_H_
#define _PHLOX_ARCH_I386_PROCESSOR_H_

/* REP NOP (PAUSE) is a good thing to insert into busy-wait loops. */
#define rep_nop()  asm volatile ("rep;nop": : :"memory")
#define cpu_relax()  rep_nop()

#define local_irqs_enable()   asm volatile ("sti": : :"memory")
#define local_irqs_disable()  asm volatile ("cli": : :"memory")

/* used in the idle loop; sti takes one instruction cycle to complete */
#define safe_halt()  asm volatile ("sti; hlt": : :"memory")

/* used when interrupts are already enabled or to shutdown the processor */
#define halt()  asm volatile ("hlt": : :"memory")

#endif
