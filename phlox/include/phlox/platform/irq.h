/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_PLATFORM_IRQ_H_
#define _PHLOX_PLATFORM_IRQ_H_

#include INC_PLATF(phlox/platform,irq.h)

/*
 * Basic hardware interrupts control interface
*/

/* interrupt acknowledge */
#define interrupt_ack(int_no)      platform_interrupt_ack(int_no)
/* interrupt enable */
#define interrupt_enable(int_no)   platform_interrupt_enable(int_no)
/* interrupt disable */
#define interrupt_disable(int_no)  platform_interrupt_disable(int_no)

#endif
