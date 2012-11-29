/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_PROCESSOR_H_
#define _PHLOX_PROCESSOR_H_

#include <phlox/compiler.h>
#include <phlox/ktypes.h>
#include <arch/cpu.h>
#include <arch/arch_bits.h>
#include <arch/arch_data.h>
#include <phlox/kargs.h>
#include <phlox/spinlock.h>
#include <phlox/arch/processor.h>

/* Bootstrap processor */
#define BOOTSTRAP_CPU  0

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


/*** Phlox memory-barrier primitives ***/
/*
 * "Memory barrier, also known as membar or memory fence, is a class of instructions
 * which cause a central processing unit (CPU) to enforce an ordering constraint on
 * memory operations issued before and after the barrier instruction. CPUs employ
 * performance optimizations that can result in out-of-order execution, including
 * memory load and store operations. Memory operation reordering normally goes
 * unnoticed within a single thread of execution, but causes unpredictable behaviour
 * in concurrent programs and device drivers unless carefully controlled. The exact
 * nature of an ordering constraint is hardware dependent, and defined by the
 * architecture's memory model. Some architectures provide multiple barriers for
 * enforcing different ordering constraints." - from Wikipedia.
*/

/** SMP memory barriers **/
/* "memory barrier" that orders both loads and stores.
 * This means loads and stores preceding the memory barrier are
 * committed to memory before any loads and stores following the
 * memory barrier.
*/
#define smp_mb()   arch_smp_mb()
/* "read memory barrier" that orders only loads. */
#define smp_rmb()  arch_smp_rmb()
/* "write memory barrier" that orders only stores. */
#define smp_wmb()  arch_smp_wmb()

/** Mandatory barriers **/
/* "memory barrier" that orders both loads and stores. */
#define mb()       arch_mb()
/* "read memory barrier" that orders only loads. */
#define rmb()      arch_rmb()
/* "write memory barrier" that orders only stores. */
#define wmb()      arch_wmb()
/*
 * All CPU memory barriers unconditionally imply compiler barriers.
 *
 * SMP memory barriers are reduced to compiler barriers on uniprocessor compiled
 * systems because it is assumed that a CPU will appear to be self-consistent,
 * and will order overlapping accesses correctly with respect to itself.
 *
 * [!] Note that SMP memory barriers _must_ be used to control the ordering of
 * references to shared memory on SMP systems, though the use of locking instead
 * is sufficient.
 *
 * Mandatory barriers should not be used to control SMP effects, since mandatory
 * barriers unnecessarily impose overhead on UP systems. They may, however, be
 * used to control MMIO effects on accesses through relaxed memory I/O windows.
 * These are required even on non-SMP systems as they affect the order in which
 * memory operations appear to a device by prohibiting both the compiler and the
 * CPU from reordering them.
*/


/* invalidate all entries in Translation Lookaside Buffer of MMU */
#define invalidate_TLB()  arch_invalidate_TLB()

/* invalidate single entry in Translation Lookaside Buffer */
#define invalidate_TLB_entry(virt_addr)  arch_invalidate_TLB_entry(virt_addr)

/* invalidate TLB entries refered by given virtual address range */
#define invalidate_TLB_range(start, size)  arch_invalidate_TLB_range(start, size)

/* invalidate list of TLB entries */
#define invalidate_TLB_list(pages, count)  arch_invalidate_TLB_list(pages, count)

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
/* this one called by processor_set_init for each cpu */
void processor_init(processor_t *p, kernel_args_t *kargs, uint32 curr_cpu);

#endif
