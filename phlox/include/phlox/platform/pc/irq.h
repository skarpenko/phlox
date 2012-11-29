/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_PLATFORM_PC_IRQ_H_
#define _PHLOX_PLATFORM_PC_IRQ_H_


/* List of interrupts used with IBM PC type systems.
 *
 *  PICU1
 *   IRQ 0   System timer
 *   IRQ 1   Keyboard
 *   IRQ 2   Cascade from PICU2 (IRQs 8-15, handled internally)
 *   IRQ 3   COM2 (default) and COM4 (user) serial ports
 *   IRQ 4   COM1 (default) and COM3 (user) serial ports
 *   IRQ 5   Parallel port 2 (LPT2) or sound card
 *   IRQ 6   Floppy disk controller
 *   IRQ 7   Parallel port 1 (LPT1) or sound card
 *
 *  PICU2
 *   IRQ 8   Real-time clock (CMOS clock)
 *   IRQ 9   Free
 *   IRQ 10  Free
 *   IRQ 11  Free
 *   IRQ 12  PS/2 connector Mouse
 *   IRQ 13  Math coprocessor / ISA
 *   IRQ 14  Primary IDE
 *   IRQ 15  Secondary IDE
*/

/* Number of IRQs in the system */
#define IRQS_NUMBER       16

/* Base vector for hardware interrupts */
#define IRQS_BASE_VECTOR  0x20

#endif
