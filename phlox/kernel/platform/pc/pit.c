/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/platform/pc/pit.h>
#include <phlox/processor.h>
#include <phlox/spinlock.h>
#include <phlox/errors.h>
#include <phlox/param.h>

/* lock for PIT ports access */
static spinlock_t pit_lock;


/* PIT initialization */
uint32 pit_init(void) {
    uint16 cntr_value = SYS_CLOCK_RATE / HZ;

    spin_init(&pit_lock); /* init spin lock */

    /* init timer */
    pit_set_counter(0, PIT_CW_MODE3 >> 1, cntr_value);

    return 0;
}

/* sets counter */
uint32 pit_set_counter(uint8 counter, uint8 mode, uint16 value) {
    uint32 irq_state;
    uint16 dport; /* data port */

    /* set correct counter value for PIT command word
     * and select right data port
    */
    switch(counter) {
       case 0:
         counter = PIT_CW_CNTR0;
         dport = PIT_CNTR0_PORT;
         break;
       case 1:
         counter = PIT_CW_CNTR1;
         dport = PIT_CNTR1_PORT;
         break;
       case 2:
         counter = PIT_CW_CNTR2;
         dport = PIT_CNTR2_PORT;
         break;
       /* counter number is invalid */
       default: return ERR_INVALID_ARGS;
    }

    /* set correct mode for PIT command word */
    switch(mode) {
       case 0:
         mode = PIT_CW_MODE0;
         break;
       case 1:
         mode = PIT_CW_MODE1;
         break;
       case 2:
         mode = PIT_CW_MODE2;
         break;
       case 3:
         mode = PIT_CW_MODE3;
         break;
       case 4:
         mode = PIT_CW_MODE4;
         break;
       case 5:
         mode = PIT_CW_MODE5;
         break;
       /* invalid mode */
       default: return ERR_INVALID_ARGS;
    }

    /* acquire spinlock  */
    irq_state = spin_lock_irqsave(&pit_lock);

    /* send command word */
    out8_p(PIT_CTRL_PORT, counter | PIT_CW_RW_LSBMSB | mode | PIT_CW_BIN);
    /* send LSB */
    out8_p(dport, value & 0x00ff);
    /* send MSB */
    out8_p(dport, (value & 0xff00) >> 8);

    /* release lock */
    spin_unlock_irqrstor(&pit_lock, irq_state);

    /* return with no error */
    return NO_ERROR;
}

/* plug/unplug counter 2 to/from PC speaker */
void pit_to_spkr(bool v) {
    if(v)
       out8_p(PIT_AUX_PORT, PIT_AUX_GATE2 | PIT_AUX_OUT2);
    else
       out8_p(PIT_AUX_PORT, ~(PIT_AUX_GATE2 | PIT_AUX_OUT2));
}
