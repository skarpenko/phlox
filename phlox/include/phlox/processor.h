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
#include <phlox/kargs.h>
#include <phlox/spinlock.h>
#include <phlox/arch/processor.h>

/* processor data */
typedef struct {
    spinlock_t        lock;  /* lock */
    arch_processor_t  arch;  /* architecture specific data */
} processor_t;

/* processor set data */
typedef struct {
    spinlock_t            lock;                         /* lock */
    arch_processor_set_t  arch;                         /* architecture specific data */
    uint32                processors_num;               /* processors count */
    processor_t           processors[SYSCFG_MAX_CPUS];  /* processors array */
} processor_set_t;

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

/* Global variables */
extern processor_set_t  ProcessorSet;  /* Processor Set */

/* Bootup processors initialization */
void processor_set_init(kernel_args_t *kargs, uint32 curr_cpu);

#endif
