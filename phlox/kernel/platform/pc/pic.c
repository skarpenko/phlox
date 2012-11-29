/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/kernel.h>
#include <phlox/processor.h>
#include <phlox/errors.h>
#include <phlox/param.h>
#include <phlox/platform/pc/pic.h>

/* lock for PICs ports access */
spinlock_t pic_lock;

/* Cached interrupt mask.
 * Bits 0-7  represents mask of master PIC.
 * Bits 8-15 represents mask of slave PIC.
 */
static uint16 cached_int_mask = 0xffff;


/* PICs initialization */
uint32 pic_init(void)
{
    spin_init(&pic_lock); /* init spin lock */

    /* Init interrupt controllers */
    out8_p(PIC_MASTER_CMD, PICM_STD_ICW1);  /* Start initialization sequence for master */
    out8_p(PIC_SLAVE_CMD,  PICS_STD_ICW1);  /* ... and for slave controllers. */
    out8_p(PIC_MASTER_IMR, PICM_STD_ICW2);  /* Base interrupt vector for master */
    out8_p(PIC_SLAVE_IMR,  PICS_STD_ICW2);  /* ... for slave. */
    out8_p(PIC_MASTER_IMR, PICM_STD_ICW3);  /* Set master PIC to operate as master */
    out8_p(PIC_SLAVE_IMR,  PICS_STD_ICW3);  /* ... and slave as slave. */
    out8_p(PIC_MASTER_IMR, PICM_STD_ICW4);  /* Set Master and slave PICs operate */
    out8_p(PIC_SLAVE_IMR,  PICS_STD_ICW4);  /* ... in i8086 mode. */

    out8_p(PIC_MASTER_IMR, PIC_OCW1_MASK_ALL);  /* Mask all interrupts on master */
    out8_p(PIC_SLAVE_IMR,  PIC_OCW1_MASK_ALL);  /* and slave */
    out8_p(PIC_MASTER_CMD, PIC_OCW2_NON_SPEC_EOI); /* Send EOI to master */
    out8_p(PIC_SLAVE_CMD,  PIC_OCW2_NON_SPEC_EOI); /* Send EOI to slave */
    /** At this point masks stored in two PICs equal to cached mask stored 
     ** in cached_int_mask variable.
     **/

    return 0;
}

/* send EOI command */
void pic_int_eoi(uint32 int_num)
{
    uint32 irq_state;
    uint32 irq_num = int_num - PIC_IRQBASE;

    /* exit if interrupt is not under PICs control */
    if(irq_num >= PIC_TOTIRQS) return;

    /* set lock for PICs access */
    irq_state = spin_lock_irqsave(&pic_lock);

    /* send EOI to slave, if interrupt controlled by it */
    if(int_num >= PIC_SLAVE_IRQBASE)
        out8_p(PIC_SLAVE_CMD, PIC_OCW2_NON_SPEC_EOI);

    /* send EOI to master */
    out8_p(PIC_MASTER_CMD, PIC_OCW2_NON_SPEC_EOI);

    /* unlock */
    spin_unlock_irqrstor(&pic_lock, irq_state);
}

/* mask interrupt */
void pic_int_mask(uint32 int_num)
{
    uint32 irq_state;
    uint32 irq_num = int_num - PIC_IRQBASE;

    /* exit if interrupt is not controlled by PICs */
    if(irq_num >= PIC_TOTIRQS) return;

    /* set lock */
    irq_state = spin_lock_irqsave(&pic_lock);

    /* set bit in cached mask */
    cached_int_mask |= BIT(irq_num);

    /* load mask to PICs */
    out8_p(PIC_MASTER_IMR, cached_int_mask & 0xff);         /* to master */
    out8_p(PIC_SLAVE_IMR,  (cached_int_mask >> 8) & 0xff);  /* to slave */

    /* unlock */
    spin_unlock_irqrstor(&pic_lock, irq_state);
}

/* unmask interrupt */
void pic_int_unmask(uint32 int_num)
{
    uint32 irq_state;
    uint32 irq_num = int_num - PIC_IRQBASE;

    /* exit if interrupt is not controlled by PICs */
    if(irq_num >= PIC_TOTIRQS) return;

    /* set lock */
    irq_state = spin_lock_irqsave(&pic_lock);

    /* clear bit in cached mask */
    cached_int_mask &= ~BIT(irq_num);

    /* if interrupt controlled by slave PIC, unmask IR line
     * of a master PIC where slave connected.
    */
    if(int_num >= PIC_SLAVE_IRQBASE)
        cached_int_mask &= ~BIT(PIC_MASTER_SLAVEIRQ);

    /* load mask to PICs */
    out8_p(PIC_MASTER_IMR, cached_int_mask & 0xff);         /* to master    */
    out8_p(PIC_SLAVE_IMR,  (cached_int_mask >> 8) & 0xff);  /* .. and slave */

    /* unlock */
    spin_unlock_irqrstor(&pic_lock, irq_state);
}

/* mask all hardware interrupts */
void pic_int_maskall(void)
{
    uint32 irq_state;

    /* set lock */
    irq_state = spin_lock_irqsave(&pic_lock);

    /* mask all IR lines in cached mask */
    cached_int_mask = 0xffff;

    /* load mask to PICs */
    out8_p(PIC_MASTER_IMR, cached_int_mask & 0xff);         /* to master    */
    out8_p(PIC_SLAVE_IMR,  (cached_int_mask >> 8) & 0xff);  /* .. and slave */

    /* unlock */
    spin_unlock_irqrstor(&pic_lock, irq_state);
}

/* unmask all hardware interrupts */
void pic_int_unmaskall(void)
{
    uint32 irq_state;

    /* set lock */
    irq_state = spin_lock_irqsave(&pic_lock);

    /* unmask all IR lines in cached mask */
    cached_int_mask = 0x0000;

    /* load mask to PICs */
    out8_p(PIC_MASTER_IMR, cached_int_mask & 0xff);         /* to master    */
    out8_p(PIC_SLAVE_IMR,  (cached_int_mask >> 8) & 0xff);  /* .. and slave */

    /* unlock */
    spin_unlock_irqrstor(&pic_lock, irq_state);
}
