/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_PLATFORM_PC_NVRAM_H_
#define _PHLOX_PLATFORM_PC_NVRAM_H_

#include <phlox/types.h>
#include <phlox/spinlock.h>
#include <phlox/processor.h>


/*
 * PC non-volatile random access memory stuff
 *
 * "Non-volatile BIOS memory refers to the memory on a personal
 * computer motherboard containing BIOS settings and sometimes the
 * code used to initialize the computer and load the operating system.
 * The non-volatile memory was historically called CMOS RAM or just
 * CMOS because it traditionally used a low-power CMOS memory chip,
 * which was powered by a small battery when the system power was off.
 * The term remains in wide use in this context, but has also grown into
 * a misnomer." - Wikipedia
*/


/*
 * Spinlock for accessing NVRAM ports. The lock must be acquired
 * before access to ports or call to routine which works with NVRAM
 * and not acquires lock before. After all operations the lock
 * must be released.
*/
extern spinlock_t nvram_lock;


#define NVRAM_ADDR  0x70  /* I/O port address for register select */
#define NVRAM_DATA  0x71  /* I/O port address for data read/write */


/*
 * Real-Time clock data registers and flags
 */
 
#define NVRAM_SECONDS      0x00  /* Seconds. BCD 00-59, Hex 00-3b
                                  * Note: Bit 7 is read only
                                  */
#define NVRAM_ASECOND      0x01  /* Second alarm. BCD 00-59, Hex 00-3b
                                  * "don't care" if 0xc0 - 0xff
                                  */
#define NVRAM_MINUTES      0x02  /* Minutes. BCD 00-59, Hex 00-3b */
#define NVRAM_AMINUTE      0x03  /* Minute alarm. BCD 00-59, Hex 00-3b
                                  * "don't care" if 0xc0 - 0xff
                                  */
#define NVRAM_HOURS        0x04  /* Hours
                                  * For 24hr mode:
                                  *    BCD 00-23, Hex 00-17
                                  * For AM/PM mode:
                                  *    BCD 01-12, Hex 01-0C if 12hr AM
                                  *    BCD 81-92, Hex 81-8C if 12hr PM
                                  */
#define NVRAM_AHOUR        0x05  /* Hour alarm. Same as NVRAM_HOURS
                                  * "don't care" if 0xc0 - 0xff
                                  */
#define NVRAM_WEEKDAY      0x06  /* Day of Week. BCD/Hex 01-07, Sunday=1 */
#define NVRAM_MONTHDAY     0x07  /* Date of month. BCD 01-31, Hex 01-1f */
#define NVRAM_MONTH        0x08  /* Month. BCD 01-12, Hex 01-0c */
#define NVRAM_YEAR         0x09  /* Year. BCD 00-99, Hex 00-63 */

/* Register A definitions */
#define NVRAM_REGA_OFF     0x0a  /* Register A offset */
#define NVRAM_REGA_UIP     0x80  /* Update in progress bit */
#define NVRAM_REGA_DIV0    0x00  /* Time base of 4.194304 MHz */
#define NVRAM_REGA_DIV1    0x10  /* Time base of 1.048576 MHz */
#define NVRAM_REGA_DIV2    0x20  /* Time base of 32.768 KHz */
#define NVRAM_REGA_IRATE0  0x00  /* Interrupt disabled */
#define NVRAM_REGA_IRATE1  0x03  /* Interrupt rate of 122 microseconds */
#define NVRAM_REGA_IRATE2  0x0f  /* Interrupt rate of 500 milliseconds */
#define NVRAM_REGA_IRATE3  0x06  /* Interrupt rate of 976.562 microseconds */

/* Register B definitions */
#define NVRAM_REGB_OFF     0x0b  /* Register B offset */
#define NVRAM_REGB_CUE     0x80  /* Cycle update enable */
#define NVRAM_REGB_PIE     0x40  /* Periodic interrupt enable */
#define NVRAM_REGB_AIE     0x20  /* Alarm interrupt enable */
#define NVRAM_REGB_UIE     0x10  /* Update ended interrupt enable */
#define NVRAM_REGB_SQWE    0x08  /* Square wave output enable */
#define NVRAM_REGB_DM      0x04  /* Data mode - 0: BCD, 1: Binary */
#define NVRAM_REGB_HM      0x02  /* Hour mode - 0: 12 hour, 1: 24 hour */
#define NVRAM_REGB_DSE     0x01  /* Daylight savings enable */

/* Register C definitions */
#define NVRAM_REGC_OFF    0x0c  /* Register C offset */
#define NVRAM_REGC_IRQF   0x80  /* Interrupt request flag */
#define NVRAM_REGC_PIF    0x40  /* Periodic interrupt flag */
#define NVRAM_REGC_AIF    0x20  /* Alarm interrupt flag */
#define NVRAM_REGC_UIF    0x10  /* Update-ended interrupt flag */

/* Register D definitions */
#define NVRAM_REGD_OFF    0x0d  /* Register D offset */
#define NVRAM_REGD_VRAM   0x80  /* Valid RAM
                                 * 1 - indicates batery power good,
                                 * 0 - if dead or disconnected.
                                 */


/*
 * Read byte from NVRAM without acquiring nvram_lock before
 */
static inline void nvram_rd_byte_nl(uint8 addr, uint8 *byte) {
    out8_p(NVRAM_ADDR, addr);
    *byte = in8_p(NVRAM_DATA);
}

/*
 * Write byte to NVRAM without acquiring nvram_lock before
 */
static inline void nvram_wr_byte_nl(uint8 addr, uint8 byte) {
    out8_p(NVRAM_ADDR, addr);
    out8_p(NVRAM_DATA, byte);
}

/*
 * Read byte from NVRAM with acquiring nvram_lock before
 */
void nvram_rd_byte(uint8 addr, uint8 *byte);

/*
 * Write byte to NVRAM with acquiring nvram_lock before
 */
void nvram_wr_byte(uint8 addr, uint8 byte);

/*
 * Read sequence of bytes from NVRAM with acquiring nvram_lock before
 */
void nvram_rd_bytes(uint8 addr, uint8 *bytes, uint8 count);

/*
 * Write sequence of bytes to NVRAM with acquiring nvram_lock before
 */
void nvram_wr_bytes(uint8 addr, uint8 *bytes, uint8 count);

#endif
