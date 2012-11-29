/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_PLATFORM_PC_PIT_H_
#define _PHLOX_PLATFORM_PC_PIT_H_

#include <phlox/types.h>
#include <phlox/spinlock.h>

/*
 *
 * Intel's Programmable Interval Timer (82C54) Stuff
 *
*/


/*
 * Spinlock for accessing PIT ports. The lock must be acquired
 * before access to ports or call to routine which works with PIT
 * and not acquires lock before. After all operations the lock
 * must be released.
*/
extern spinlock_t pit_lock;


/* Clock rate for the PIT */
#define SYS_CLOCK_RATE 1193182  /* Hz */


/*** I/O ports ***/

/* Counters data ports */
#define PIT_CNTR0_PORT  0x40
#define PIT_CNTR1_PORT  0x41
#define PIT_CNTR2_PORT  0x42
/* Control port */
#define PIT_CTRL_PORT   0x43
/* Auxiliary port */
#define PIT_AUX_PORT    0x61

/*** Control word bits ***/

/* Counter selection or Read-Back command */
#define PIT_CW_CNTR0      0x00  /* Counter 0 */
#define PIT_CW_CNTR1      0x40  /* Counter 1 */
#define PIT_CW_CNTR2      0x80  /* Counter 2 */
#define PIT_CW_RBCMD      0xc0  /* Read-Back command */
/* R/W Operation type or Latch command */
#define PIT_CW_LTCHCMD    0x00  /* Latch command */
#define PIT_CW_RW_LSB     0x10  /* R/W LSB only */
#define PIT_CW_RW_MSB     0x20  /* R/W MSB only */
#define PIT_CW_RW_LSBMSB  0x30  /* R/W LSB, then MSB */
/* Mode */
#define PIT_CW_MODE0      0x00  /* Interrupt on terminal count */
#define PIT_CW_MODE1      0x02  /* Hardware retriggerable one-shot */
#define PIT_CW_MODE2      0x04  /* Rate generator */
#define PIT_CW_MODE3      0x06  /* Square wave mode */
#define PIT_CW_MODE4      0x08  /* Software triggered strobe */
#define PIT_CW_MODE5      0x0a  /* Hardware triggered strobe (Retriggerable) */
/* Counter type */
#define PIT_CW_BIN        0x00  /* Binary counter (16-bits) */
#define PIT_CW_BCD        0x01  /* Binary coded decimal counter (4 Decades) */

/* Read-Back Command Bits */
#define PIT_CW_RBCMD_COUNT     0x20  /* if =0 Latch count of selected counters */
#define PIT_CW_RBCMD_STATUS    0x10  /* if =0 Latch status of selected counters */
#define PIT_CW_RBCMD_CNTR0     0x02  /* Counter 0 */
#define PIT_CW_RBCMD_CNTR1     0x04  /* Counter 1 */
#define PIT_CW_RBCMD_CNTR2     0x08  /* Counter 2 */
/* Read-Back Command Status Byte */
#define PIT_RBCMD_STAT_OUTPUT     0x80  /* Out pin state */
#define PIT_RBCMD_STAT_NULLCNT    0x40  /* Null count */
/* R/W mode */
#define PIT_RBCMD_STAT_RW_LSB     0x10
#define PIT_RBCMD_STAT_RW_MSB     0x20
#define PIT_RBCMD_STAT_RW_LSBMSB  0x30
/* Counter programmed mode */
#define PIT_RBCMD_STAT_MODE0      0x00
#define PIT_RBCMD_STAT_MODE1      0x02
#define PIT_RBCMD_STAT_MODE2      0x04
#define PIT_RBCMD_STAT_MODE3      0x06
#define PIT_RBCMD_STAT_MODE4      0x08
#define PIT_RBCMD_STAT_MODE5      0x0a
/* Counter type */
#define PIT_RBCMD_STAT_BIN        0x00
#define PIT_RBCMD_STAT_BCD        0x01

/* Auxiliary control port for timer 2 */
#define PIT_AUX_GATE2  0x01  /* aux port, PIT gate 2 input */
#define PIT_AUX_OUT2   0x02  /* aux port, PIT clock out 2 enable */
/*
 * Note: For more information refer to Intel's Manual
 * "82C54 CHMOS PROGRAMMABLE INTERVAL TIMER"
*/

/*
 * Called once during kernel init
*/
uint32 pit_init(void);

/*
 * Sets counter to count
 * This routine acquires spinlock before accessing PIT ports.
 *
 * Input: counter = 0 - 2;
 *        mode = 0 - 5;
 *        value - initial counter value.
 *
 * Note: counter and mode is not the same values defined by
 * PIT_CW_* constants.
 *
 * Returns: NO_ERROR or ERR_INVALID_ARGS.
*/
uint32 pit_set_counter(uint8 counter, uint8 mode, uint16 value);

/*
 * Plugs or unplugs counter 2 output to/from
 * PC speaker. Input is true or false respectively.
 * This routine acquires spinlock before accessing PIT ports.
*/
void pit_to_spkr(bool v);

#endif
