/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_INTERRUPT_H_
#define _PHLOX_INTERRUPT_H_

#include <phlox/types.h>
#include <phlox/kargs.h>
#include <phlox/platform/irq.h>
#include <phlox/arch/interrupt.h>

/* Hardware interrupts count */
#define HW_INTERRUPTS_NUM  IRQS_NUMBER

/* Flags returned by interrupt handlers */
#define INT_FLAGS_NOFLAGS  0x00000000  /* no flags set */
#define INT_FLAGS_RESCHED  0x00000001  /* reschedule needed */

/* Hardware interrupt handling routine */
typedef flags_t (hw_int_handler_t)(void *);

/*
 * Interrupt handling initialization routine.
 * Called during kernel startup.
 */
status_t interrupt_init(kernel_args_t *kargs);

/*
 * Called during interrupt handling when
 * hardware interrupt occurs.
 * (returns flags collected from handlers)
 */
flags_t handle_hw_interrupt(uint hw_vector);

/*
 * Enable hardware interrupt
 * Returns NO_ERROR on success.
 */
result_t hw_interrupt_enable(uint hw_int);

/*
 * Disable hardware interrupt
 * Returns NO_ERROR on success.
 */
result_t hw_interrupt_disable(uint hw_int);

/*
 * Set hardware interrupt handler
 * Params: hw_vector - hardware interrupt vector;
 *         handler   - interrupt handling routine;
 *         name      - handler's name;
 *         data      - additional data passed to handling routine.
 * Returns NO_ERROR on success.
 */
result_t set_hw_interrupt_handler(uint hw_vector, hw_int_handler_t *handler,
                                  const char *name, void *data);

/*
 * Remove hardware interrupt handler
 * Returns NO_ERROR on success.
 */
result_t remove_hw_interrupt_handler(uint hw_vector, hw_int_handler_t *handler,
                                     void *data);

#endif
