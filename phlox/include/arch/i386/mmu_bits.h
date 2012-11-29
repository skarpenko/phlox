/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_I386_MMU_BITS_H_
#define _PHLOX_ARCH_I386_MMU_BITS_H_

/*
* Page Directory entry format
*
* 31                12 11    9 8 7 6 5 4 3 2 1 0
* -----------------------------------------------
* |                   | Avail-|G|P|0|A|P|P|U|R|P|
* |PAGE TABLE ADDRESS | able  | |S| | |C|W|/|/| |
* |                   |       | | | | |D|T|S|W| |
* -----------------------------------------------
* P   - Present                    PCD - Page Cache Disable (486+)
* R/W - Readable/Writable (0/1)    A   - Accessed
* U/S - User/Supervisor (1/0)      PS  - Page Size (P5+)
* PWT - Page Write Throught (486+) G   - Global (P6+)
*/

/*
* Page Table entry format
*
* 31                12 11    9 8 7 6 5 4 3 2 1 0
* -----------------------------------------------
* |                   | Avail-|G|0|D|A|P|P|U|R|P|
* |PAGE FRAME ADDRESS | able  | | | | |C|W|/|/| |
* |                   |       | | | | |D|T|S|W| |
* -----------------------------------------------
* P   - Present                    PCD - Page Cache Disable (486+)
* R/W - Readable/Writable (0/1)    A   - Accessed
* U/S - User/Supervisor (1/0)      D   - Dirty
* PWT - Page Write Throught (486+) G   - Global (P6+)
*/

/*
* Page Directory entry format for 4Mb pages (P5+)
*
* 31       22 21    12 11    9 8 7 6 5 4 3 2 1 0
* -----------------------------------------------
* |PAGE FRAME|        | Avail-|G|P|D|A|P|P|U|R|P|
* | ADDRESS  |Reserved| able  | |S| | |C|W|/|/| |
* |          |        |       | | | | |D|T|S|W| |
* -----------------------------------------------
* P   - Present                    A   - Accessed
* R/W - Readable/Writable (0/1)    D   - Dirty
* U/S - User/Supervisor (1/0)      PS  - Page Size (P5+)
* PWT - Page Write Throught (486+) G   - Global (P6+)
* PCD - Page Cache Disable (486+)
*/

/** Paging with Physical Address Extension (P6+) **/
/*
* Page Directory Pointer Table Entry (P6+)
*
* 31          12 11  9 8               0
* ---------------------------------------
* | Base address|     | | | | |P|P| | | |
* | of page dir.|AVAIL|  res  |C|W|res|1| +0
* |  (0..19)    |     | | | | |D|T| | | |
* ---------------------------------------
* |                              | ...  |
* |     RESERVED ALWAYS 0        | (20..| +4
* |                              |  23) |
* ---------------------------------------
* 63                           36 35   32
*
* AVAIL - Available
* res   - Reserved
*/

/*
* Page Directory Entry for 4K Pages (P6+)
*
* 31          12 11  9 8               0
* ---------------------------------------
* | Base address|     | | | | |P|P|U|R| |
* | of page tab.|AVAIL|0|0|0|A|C|W|/|/|P| +0
* |  (0..19)    |     | | | | |D|T|S|W| |
* ---------------------------------------
* |                              | ...  |
* |     RESERVED ALWAYS 0        | (20..| +4
* |                              |  23) |
* ---------------------------------------
* 63                           36 35   32
*
* AVAIL - Available
*/

/*
* Page Table Entry for 4K Pages (P6+)
*
* 31          12 11  9 8               0
* ---------------------------------------
* | Base address|     | | | | |P|P|U|R| |
* |   of page   |AVAIL|G|0|D|A|C|W|/|/|P| +0
* |  (0..19)    |     | | | | |D|T|S|W| |
* ---------------------------------------
* |                              | ...  |
* |     RESERVED ALWAYS 0        | (20..| +4
* |                              |  23) |
* ---------------------------------------
* 63                           36 35   32
*
* AVAIL - Available
*/

/*
* Page Directory Entry for 2M Pages (P6+)
*
* 31          12 11  9 8               0
* ---------------------------------------
* | Base address|     | | | | |P|P|U|R| |
* |   of page   |AVAIL|G|1|D|A|C|W|/|/|P| +0
* |  (0..19)    |     | | | | |D|T|S|W| |
* ---------------------------------------
* |                              | ...  |
* |     RESERVED ALWAYS 0        | (20..| +4
* |                              |  23) |
* ---------------------------------------
* 63                           36 35   32
*
* AVAIL - Available
*/

/* Attributes byte */
#define X86_PG_ATB_P    0x00000001  /* Present                    */
#define X86_PG_ATB_W    0x00000002  /* Writable                   */
#define X86_PG_ATB_R    0x00000000  /* Read Only                  */
#define X86_PG_ATB_U    0x00000004  /* User                       */
#define X86_PG_ATB_S    0x00000000  /* Supervisor                 */
#define X86_PG_ATB_PWT  0x00000008  /* Page Write Throught (486+) */
#define X86_PG_ATB_PCD  0x00000010  /* Page Cache Disable (486+)  */
#define X86_PG_ATB_A    0x00000020  /* Accessed                   */
#define X86_PG_ATB_D    0x00000040  /* Dirty                      */
#define X86_PG_ATB_PS   0x00000080  /* Page Size (P5+)            */
/* Avail byte */
#define X86_PG_AVB_G    0x00000100  /* Global (P6+)               */

#endif
