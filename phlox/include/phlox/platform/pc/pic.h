/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_PLATFORM_PC_PIC_H_
#define _PHLOX_PLATFORM_PC_PIC_H_

#include <phlox/types.h>
#include <phlox/spinlock.h>
#include <phlox/platform/pc/irq.h>

/*
 *
 * Intel's Programmable Interrupt Controller (8259A) Stuff
 *
*/


/*
 * Spinlock for accessing PICs ports. The lock must be acquired
 * before access to ports or call to routine which works with PICs
 * and not acquires lock before. After all operations the lock
 * must be released.
*/
extern spinlock_t pic_lock;


#define PIC_NIRQS    0x08                     /* IRQs number per PIC       */
#define PIC_NPICS    0x02                     /* Number of PICs            */
#define PIC_TOTIRQS  (PIC_NPICS * PIC_NIRQS)  /* Total number of IRQs      */
#define PIC_IRQBASE  IRQS_BASE_VECTOR         /* Base vector for both PICs */

/* Base interrupt vector for master PIC */
#define PIC_MASTER_IRQBASE   PIC_IRQBASE
/* Base interrupt vector for slave PIC */
#define PIC_SLAVE_IRQBASE    (PIC_IRQBASE + PIC_NIRQS)
/* IR line of master PIC where slave connected */
#define PIC_MASTER_SLAVEIRQ  0x02

/*
 * The following definitions used to locate PICs in the system
*/

#define PIC_IO_BASE      0x20   /* I/O base port */
#define PIC_OFF_CMD      0x00   /* Offset of CMD port */
#define PIC_OFF_IMR      0x01   /* Offset of IMR port */
#define PIC_IO_SIZE      0x80   /* I/O range size between PICs */

/* Master PIC ports */
#define PIC_MASTER_CMD   (PIC_IO_BASE + PIC_OFF_CMD)
#define PIC_MASTER_IMR   (PIC_IO_BASE + PIC_OFF_IMR)
/* Slave PIC ports */
#define PIC_SLAVE_CMD    (PIC_IO_BASE + PIC_IO_SIZE + PIC_OFF_CMD)
#define PIC_SLAVE_IMR    (PIC_IO_BASE + PIC_IO_SIZE + PIC_OFF_IMR)
/*
 * Note: ICW1, OCW2 and OCW3 goes to CMD port. ICW2, ICW3, ICW4,
 * and OCW1 goes to IMR port.
*/


/*
 * The following banks of definitions are used to define
 * the fields of the various Initialization Command Words (ICWs)
 * for PICs initialization.
 */

/*
 * ICW1
*/

#define PIC_ICW1        0x10  /* Common ICW1 field */
#define PIC_ICW1_IC4    0x01  /* ICW4 needed */
#define PIC_ICW1_NOIC4  0x00  /* No ICW4 needed */
#define PIC_ICW1_SNGL   0x02  /* Single */
#define PIC_ICW1_CSCD   0x00  /* Cascade mode */
#define PIC_ICW1_ADI4   0x04  /* Call address interval 4 */
#define PIC_ICW1_ADI8   0x00  /* Call address interval 8 */
#define PIC_ICW1_LVTM   0x08  /* Level triggered mode */
#define PIC_ICW1_EDTM   0x00  /* Edge triggered mode */

/*
 * ICW2
*/

#define PICM_ICW2_VECBASE   (PIC_MASTER_IRQBASE)  /* Vector base for master PIC */
#define PICS_ICW2_VECBASE   (PIC_SLAVE_IRQBASE)   /* Vector base for slave PIC  */

/*
 * ICW3
*/

/* For master PIC. Defines inputs with
 * slave PIC connected
*/
#define PICM_ICW3_SLAVE_ON_IR0  0x01
#define PICM_ICW3_SLAVE_ON_IR1  0x02
#define PICM_ICW3_SLAVE_ON_IR2  0x04
#define PICM_ICW3_SLAVE_ON_IR3  0x08
#define PICM_ICW3_SLAVE_ON_IR4  0x10
#define PICM_ICW3_SLAVE_ON_IR5  0x20
#define PICM_ICW3_SLAVE_ON_IR6  0x40
#define PICM_ICW3_SLAVE_ON_IR7  0x80
/* For slave PIC. Defines slave PIC ID */
#define PICS_ICW3_I_AM_SLAVE_0  0x00
#define PICS_ICW3_I_AM_SLAVE_1  0x01
#define PICS_ICW3_I_AM_SLAVE_2  0x02
#define PICS_ICW3_I_AM_SLAVE_3  0x03
#define PICS_ICW3_I_AM_SLAVE_4  0x04
#define PICS_ICW3_I_AM_SLAVE_5  0x05
#define PICS_ICW3_I_AM_SLAVE_6  0x06
#define PICS_ICW3_I_AM_SLAVE_7  0x07

/*
 * ICW4
*/

#define PIC_ICW4_8086_MOD  0x01  /* 8086/8088 Mode */
#define PIC_ICW4_MCS_MOD   0x00  /* MCS-80/85 Mode */
#define PIC_ICW4_AUTOEOI   0x02  /* Auto EOI */
#define PIC_ICW4_NRMLEOI   0x00  /* Normal EOI */
#define PIC_ICW4_MASTER    0x04  /* Selects master PIC for buffered mode */
#define PIC_ICW4_SLAVE     0x00  /* Selects slave PIC for buffered mode */
#define PIC_ICW4_NONBUF    0x00  /* Nonbuffered mode */
#define PIC_ICW4_BUF       0x08  /* Buffered mode */
#define PIC_ICW4_ENA_SFNM  0x10  /* Special fully nested mode */
#define PIC_ICW4_DIS_SFNM  0x00  /* Not special fully nested mode */

/* Buffered mode / master */
#define PIC_ICW4_BUFMR     (PIC_ICW4_BUF | PIC_ICW4_MASTER)
/* Buffered mode / slave */
#define PIC_ICW4_BUFSL     (PIC_ICW4_BUF | PIC_ICW4_SLAVE)


/*
 * The following banks of definitions are used to define
 * the fields of the various Operation Command Words (OCWs).
 */

/*
 * OCW1
*/

#define PIC_OCW1_MASK_ALL  0xff  /* Mask all interrupt requests */
#define PIC_OCW1_MASK_IR0  0x01  /* Mask IR0 */
#define PIC_OCW1_MASK_IR1  0x02  /* Mask IR1 */
#define PIC_OCW1_MASK_IR2  0x04  /* Mask IR2 */
#define PIC_OCW1_MASK_IR3  0x08  /* Mask IR3 */
#define PIC_OCW1_MASK_IR4  0x10  /* Mask IR4 */
#define PIC_OCW1_MASK_IR5  0x20  /* Mask IR5 */
#define PIC_OCW1_MASK_IR6  0x40  /* Mask IR6 */
#define PIC_OCW1_MASK_IR7  0x80  /* Mask IR7 */

/*
 * OCW2
*/

/* IR level to be acted upon when
 * the SL bit is active
*/
#define PIC_OCW2_ACTED_IR0  0x00
#define PIC_OCW2_ACTED_IR1  0x01
#define PIC_OCW2_ACTED_IR2  0x02
#define PIC_OCW2_ACTED_IR3  0x03
#define PIC_OCW2_ACTED_IR4  0x04
#define PIC_OCW2_ACTED_IR5  0x05
#define PIC_OCW2_ACTED_IR6  0x06
#define PIC_OCW2_ACTED_IR7  0x07
#define PIC_OCW2_EOI        0x20  /* End Of Interrupt */
#define PIC_OCW2_SL         0x40  /* Select level */
#define PIC_OCW2_ROT        0x80  /* Rotation */

/* Non-specific EOI command */
#define PIC_OCW2_NON_SPEC_EOI  (PIC_OCW2_EOI)
/* Specific EOI command */
#define PIC_OCW2_SPECIFIC_EOI  (PIC_OCW2_SL | PIC_OCW2_EOI)
/* Rotate on non-specific EOI command */
#define PIC_OCW2_ROT_NON_SPEC  (PIC_OCW2_ROT | PIC_OCW2_EOI)
/* Rotate in automatic EOI mode (Set) */
#define PIC_OCW2_SET_ROT_AEOI  (PIC_OCW2_ROT)
/* Rotate in automatic EOI mode (Clear) */
#define PIC_OCW2_CLR_ROT_AEOI  (0x00)
/* Rotate on specific EOI command */
#define PIC_OCW2_ROT_SPEC_EOI  (PIC_OCW2_ROT | PIC_OCW2_SL | PIC_OCW2_EOI)
/* Set priority command */
#define PIC_OCW2_SET_PRIORITY  (PIC_OCW2_ROT | PIC_OCW2_SL)
/* No operation */
#define PIC_OCW2_NO_OPERATION  (PIC_OCW2_SL)


/*
 * OCW3
*/

#define PIC_OCW3          0x08  /* Common OCW3 field */
#define PIC_OCW3_RR       0x02  /* Read register command */
#define PIC_OCW3_RIR      0x00  /* Read Interrupt Request Register */
#define PIC_OCW3_RIS      0x01  /* Read In-Service Register */
#define PIC_OCW3_POLL     0x04  /* Poll command */
#define PIC_OCW3_NOPOLL   0x00  /* No poll command */
#define PIC_OCW3_SMM      0x20  /* Special mask mode */
#define PIC_OCW3_ESMM     0x40  /* Enable special mask mode */

/* Read IR register on next ~RD pulse */
#define PIC_OCW3_READ_IR  (PIC_OCW3_RR | PIC_OCW3_RIR)
/* Read IS register on next ~RD pulse */
#define PIC_OCW3_READ_IS  (PIC_OCW3_RR | PIC_OCW3_RIS)
/* Set special mask */
#define PIC_OCW3_SET_SM   (PIC_OCW3_ESMM | PIC_OCW3_SMM)
/* Reset special mask */
#define PIC_OCW3_CLR_SM   (PIC_OCW3_ESMM)
/*
 * Note: For more information refer to Intel's Manual
 * "8259A PROGRAMMABLE INTERRUPT CONTROLLER (8259A/8259A-2)"
*/


/*
 *  Standard PIC initialization values for PCs
*/

/* For master PIC */
#define PICM_STD_ICW1  (PIC_ICW1 | PIC_ICW1_EDTM | PIC_ICW1_ADI8 | \
                        PIC_ICW1_CSCD | PIC_ICW1_IC4)
#define PICM_STD_ICW2  (PICM_ICW2_VECBASE)
#define PICM_STD_ICW3  (PICM_ICW3_SLAVE_ON_IR2)
#define PICM_STD_ICW4  (PIC_ICW4_DIS_SFNM | PIC_ICW4_NONBUF | \
                        PIC_ICW4_NRMLEOI | PIC_ICW4_8086_MOD)
/* For slave PIC */
#define PICS_STD_ICW1  (PIC_ICW1 | PIC_ICW1_EDTM | PIC_ICW1_ADI8 | \
                        PIC_ICW1_CSCD | PIC_ICW1_IC4)
#define PICS_STD_ICW2  (PICS_ICW2_VECBASE)
#define PICS_STD_ICW3  (PICS_ICW3_I_AM_SLAVE_2)
#define PICS_STD_ICW4  (PIC_ICW4_DIS_SFNM | PIC_ICW4_NONBUF | \
                        PIC_ICW4_NRMLEOI | PIC_ICW4_8086_MOD)


/*
 * Set of routines to operate with PICs and controlled interrupts.
 */

/*
 * This one called during system init to get PICs ready to work
*/
uint32 pic_init(void);

/*
 * Sends End Of Interrupt command to Master or Master and Slave PICs
 * depending on specified interrupt number.
 * This routine acquires spinlock before accessing PICs.
 * Note: Interrupts which is out of PICs control ignored.
*/
void pic_int_eoi(uint32 int_num);

/*
 * Masks specified hardware interrupt.
 * This routine acquires spinlock before
 * accessing PICs.
 * Note: Interrupts which is out of PICs control ignored.
*/
void pic_int_mask(uint32 int_num);

/*
 * Unasks specified hardware interrupt.
 * This routine acquires spinlock before accessing PICs.
 * Note: Interrupts which is out of PICs control ignored.
*/
void pic_int_unmask(uint32 int_num);

/*
 * Masks all hardware interrupts controlled by PICs.
 * This routine acquires spinlock before accessing PICs.
*/
void pic_int_maskall(void);

/*
 * Unmasks all hardware interrupts controlled by PICs.
 * This routine acquires spinlock before accessing PICs.
*/
void pic_int_unmaskall(void);

#endif
