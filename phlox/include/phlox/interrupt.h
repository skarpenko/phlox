/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
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

/* Hardware interrupt handling routine */
typedef uint32 (hw_int_handler_t)(void *);

/*
 * Interrupt handling initialization routine.
 * Called during kernel startup.
 */
uint32 interrupt_init(kernel_args_t *kargs);

/*
 * Called during interrupt handling when
 * hardware interrupt occurs.
 * (returns flags collected from handlers)
 */
uint32 handle_hw_interrupt(uint32 hw_vector);

/*
 * Enable hardware interrupt
 * Returns NO_ERROR on success.
 */
uint32 hw_interrupt_enable(uint32 hw_int);

/*
 * Disable hardware interrupt
 * Returns NO_ERROR on success.
 */
uint32 hw_interrupt_disable(uint32 hw_int);

/*
 * Set hardware interrupt handler
 * Params: hw_vector - hardware interrupt vector;
 *         handler   - interrupt handling routine;
 *         name      - handler's name;
 *         data      - additional data passed to handling routine.
 * Returns NO_ERROR on success.
 */
uint32 set_hw_interrupt_handler(uint32 hw_vector, hw_int_handler_t *handler,
                                const char *name, void *data);

/*
 * Remove hardware interrupt handler
 * Returns NO_ERROR on success.
 */
uint32 remove_hw_interrupt_handler(uint32 hw_vector, hw_int_handler_t *handler,
                                   void *data);

#endif
