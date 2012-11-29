/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/platform/pc/nvram.h>

/* lock for NVRAM ports access */
spinlock_t nvram_lock;


/* read byte */
void nvram_rd_byte(uint8 addr, uint8 *byte)
{
    uint32 irqs_state;

    /* acquire lock */
    irqs_state = spin_lock_irqsave(&nvram_lock);
    /* read */
    nvram_rd_byte_nl(addr, byte);
    /* release lock */
    spin_unlock_irqrstor(&nvram_lock, irqs_state);
}

/* write byte */
void nvram_wr_byte(uint8 addr, uint8 byte)
{
    uint32 irqs_state;

    /* acquire lock */
    irqs_state = spin_lock_irqsave(&nvram_lock);
    /* write */
    nvram_wr_byte_nl(addr, byte);
    /* release lock */
    spin_unlock_irqrstor(&nvram_lock, irqs_state);
}

/* read sequence of bytes */
void nvram_rd_bytes(uint8 addr, uint8 *bytes, uint8 count)
{
    uint32 irqs_state;
    uint8 i;

    /* acquire lock */
    irqs_state = spin_lock_irqsave(&nvram_lock);

    /* read sequence of bytes */
    for(i=0; i < count && addr + i < 256; i++)
       nvram_rd_byte_nl(addr + i, &bytes[i]);

    /* release lock */
    spin_unlock_irqrstor(&nvram_lock, irqs_state);
}

/* write sequence of bytes */
void nvram_wr_bytes(uint8 addr, uint8 *bytes, uint8 count)
{
    uint32 irqs_state;
    uint8 i;

    /* acquire lock */
    irqs_state = spin_lock_irqsave(&nvram_lock);

    /* write sequence of bytes */
    for(i=0; i < count && addr + i < 256; i++)
       nvram_wr_byte_nl(addr + i, bytes[i]);

    /* release lock */
    spin_unlock_irqrstor(&nvram_lock, irqs_state);
}
