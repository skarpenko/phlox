/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_I386_CPU_BITS_H_
#define _PHLOX_ARCH_I386_CPU_BITS_H_

/*
* Control registers and flags
*
* CR0
* 31       18  16    5 4 3 2 1 0
* -------------------------------
* |P|C|N|  |A| |W|  |N|E|T|E|M|P|
* |G|D|W|..|M| |P|..|E|T|S|M|P|E|
* | | | |  | | | |  | | | | | | |
* -------------------------------
* CR1
* 31                           0
* -------------------------------
* |                             |
* |                             |
* |                             |
* -------------------------------
* CR2
* 31                           0
* -------------------------------
* |                             |
* | PAGE FAULT LINEAR ADDRESS   |
* |                             |
* -------------------------------
* CR3
* 31             12    4 3     0
* -------------------------------
* |      PAGE     |   |P|P| | | |
* |    DIRECTORY  |...|C|W| | | |
* |      BASE     |   |D|T| | | |
* -------------------------------
* CR4
* 31           8 7 6 5 4 3 2 1 0
* -------------------------------
* |           |P|P|M|P|P|D|T|P|V|
* |...........|C|G|C|A|S|E|S|V|M|
* |           |E|E|E|E|E| |D|I|E|
* -------------------------------
* PE - Protection Enable              PCD - Page-Level Cache Disable (486+)
* MP - Monitor Processor Extension    PWT - Page-Level Writes Throught (486+)
* EM - Processor Extension Emulated   VME - Virtual-8086 Mode Extensions
* TS - Task Switch                    PVI - Protected-Mode Virtual Interrupts
* ET - Extension Type (486+)          TSD - Time Stamp Disable
* NE - Numeric Error (486+)           DE  - Debugging Extensions
* WP - Write Protect                  PSE - Page Size Extension
* AM - Alignment Mask                 PAE - Physical Address Extension
* NW - Not Writethrought              MCE - Machine-Check Enable (P5+)
* CD - Cache Disable                  PGE - Paging Global Extensions (P6+)
* PG - Paging Enable                  PCE - Performance-monitoring
*                                           Counter Enable
*/

/* CR0 Bits */
#define X86_CR0_PE   0x00000001
#define X86_CR0_MP   0x00000002
#define X86_CR0_EM   0x00000004
#define X86_CR0_TS   0x00000008
#define X86_CR0_ET   0x00000010
#define X86_CR0_NE   0x00000020
#define X86_CR0_WP   0x00010000
#define X86_CR0_AM   0x00040000
#define X86_CR0_NW   0x20000000
#define X86_CR0_CD   0x40000000
#define X86_CR0_PG   0x80000000

/* CR3 Bits */
#define X86_CR3_PWT  0x00000008
#define X86_CR3_PCD  0x00000010

/* CR4 Bits */
#define X86_CR4_VME  0x00000001
#define X86_CR4_PVI  0x00000002
#define X86_CR4_TSD  0x00000004
#define X86_CR4_DE   0x00000008
#define X86_CR4_PSE  0x00000010
#define X86_CR4_PAE  0x00000020
#define X86_CR4_MCE  0x00000040
#define X86_CR4_PGE  0x00000080
#define X86_CR4_PCE  0x00000100

/*
* EFLAGS Register
*
* 31      21          15             8               0
* -----------------------------------------------------
* |       | |V|V| | | | | | | | | | | | | | | | | | | |
* |   0   |I|I|I|A|V|R|0|N|IO |O|D|I|T|S|Z|0|A|0|P|1|C|
* |       |D|P|F|C|M|F| |T| PL|F|F|F|F|F|F| |F| |F| |F|
* |       |x|x|x|x|x|x| |x|x|x|s|c|x|x|s|s| |s| |s| |s|
* -----------------------------------------------------
*
* s - STATUS FLAG, c - CONTROL FLAG, x - SYSTEM FLAG
*
* ID   - ID Flag                         DF - Direction Flag
* VIP  - Virtual Interrupt Pending       IF - Interrupt-enable Flag
* VIF  - Virtual Interrupt Flag          TF - Trap Flag
* AC   - Alignment Check                 SF - Sign Flag
* VM   - Virtual 8086 Mode               ZF - Zero Flag
* RF   - Resume Flag                     AF - Auxiliary Flag
* IOPL - Input/Output Privilege Level    PF - Parity Flag
* NT   - Nested Task Flag                CF - Carry Flag
* OF   - Overflow Flag
*/

/* EFLAGS Bits */
#define X86_EFLAGS_CF      0x00000001
#define X86_EFLAGS_PF      0x00000004
#define X86_EFLAGS_AF      0x00000010
#define X86_EFLAGS_ZF      0x00000040
#define X86_EFLAGS_SF      0x00000080
#define X86_EFLAGS_TF      0x00000100
#define X86_EFLAGS_IF      0x00000200
#define X86_EFLAGS_DF      0x00000400
#define X86_EFLAGS_OF      0x00000800
#define X86_EFLAGS_IOPL_0  0x00000000
#define X86_EFLAGS_IOPL_1  0x00001000
#define X86_EFLAGS_IOPL_2  0x00002000
#define X86_EFLAGS_IOPL_3  0x00003000
#define X86_EFLAGS_NT      0x00004000
#define X86_EFLAGS_RF      0x00010000
#define X86_EFLAGS_VM      0x00020000
#define X86_EFLAGS_AC      0x00040000
#define X86_EFLAGS_VIF     0x00080000
#define X86_EFLAGS_VIP     0x00100000
#define X86_EFLAGS_ID      0x00200000

/* FLAGS Bits */
#define X86_FLAGS_CF       0x0001
#define X86_FLAGS_PF       0x0004
#define X86_FLAGS_AF       0x0010
#define X86_FLAGS_ZF       0x0040
#define X86_FLAGS_SF       0x0080
#define X86_FLAGS_TF       0x0100
#define X86_FLAGS_IF       0x0200
#define X86_FLAGS_DF       0x0400
#define X86_FLAGS_OF       0x0800
#define X86_FLAGS_IOPL_0   0x0000
#define X86_FLAGS_IOPL_1   0x1000
#define X86_FLAGS_IOPL_2   0x2000
#define X86_FLAGS_IOPL_3   0x3000
#define X86_FLAGS_NT       0x4000

/*
* Debug registers and flags
*
* DR0
* 31                                                              0
* -----------------------------------------------------------------
* |                BREAKPOINT 0 LINEAR ADDRESS                    |
* |                                                               |
* -----------------------------------------------------------------
* DR1
* 31                                                              0
* -----------------------------------------------------------------
* |                BREAKPOINT 1 LINEAR ADDRESS                    |
* |                                                               |
* -----------------------------------------------------------------
* DR2
* 31                                                              0
* -----------------------------------------------------------------
* |                BREAKPOINT 2 LINEAR ADDRESS                    |
* |                                                               |
* -----------------------------------------------------------------
* DR3
* 31                                                              0
* -----------------------------------------------------------------
* |                BREAKPOINT 3 LINEAR ADDRESS                    |
* |                                                               |
* -----------------------------------------------------------------
* DR4
* 31                                                              0
* -----------------------------------------------------------------
* |                          RESERVED                             |
* |                                                               |
* -----------------------------------------------------------------
* DR5
* 31                                                              0
* -----------------------------------------------------------------
* |                          RESERVED                             |
* |                                                               |
* -----------------------------------------------------------------
* DR6
* 31              23              15               7              0
* -----------------------------------------------------------------
* |0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|B|B|B|0|0|0|0|0|0|0|0|0|B|B|B|B|
* |1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|T|S|D| |1|1|1|1|1|1|1|1|3|2|1|0|
* -----------------------------------------------------------------
* DR7
* 31              23              15               7              0
* -----------------------------------------------------------------
* |LEN|R|W|LEN|R|W|LEN|R|W|LEN|R|W|0|0|G|0|0|0|G|L|G|L|G|L|G|L|G|L|
* | 3 |3|3| 2 |2|2| 1 |1|1| 0 |0|0| | |D| | |1|E|E|3|3|2|2|1|1|0|0|
* -----------------------------------------------------------------
*
* 0 - 386, 486
* 1- P5+
*
* GD - Global Debug Register Access Detect.
*      (If GD=1 INT 1 occurs when debug registers are accessed.)
*
* Debug Control Register (DR7)
*
* The debug control register both helps to define the debug conditions
* and selectively enables and disables those conditions.
*
* For each address in registers DR0-DR3, the corresponding fields R/W0
* through R/W3 specify the type of action that should cause a breakpoint. The
* processor interprets these bits as follows:
*
*    00 - Break on instruction execution only
*    01 - Break on data writes only
*    10 - undefined
*    11 - Break on data reads or writes but not instruction fetches
*
* Fields LEN0 through LEN3 specify the length of data item to be monitored. A
* length of 1, 2, or 4 bytes may be specified. The values of the length fields
* are interpreted as follows:
*
*    00 - one-byte length
*    01 - two-byte length
*    10 - undefined
*    11 - four-byte length
*
* If RWn is 00 (instruction execution), then LENn should also be 00. Any other
* length is undefined.
*
* The low-order eight bits of DR7 (L0 through L3 and G0 through G3)
* selectively enable the four address breakpoint conditions. There are two
* levels of enabling: the local (L0 through L3) and global (G0 through G3)
* levels. The local enable bits are automatically reset by the processor at
* every task switch to avoid unwanted breakpoint conditions in the new task.
* The global enable bits are not reset by a task switch; therefore, they can
* be used for conditions that are global to all tasks.
*
* The LE and GE bits control the "exact data breakpoint match" feature of the
* processor. If either LE or GE is set, the processor slows execution so that
* data breakpoints are reported on the instruction that causes them. It is
* recommended that one of these bits be set whenever data breakpoints are
* armed. The processor clears LE at a task switch but does not clear GE.
*
*
*
* Debug Status Register (DR6)
*
* The debug status register permits the debugger to determine which debug
* conditions have occurred.
*
* When the processor detects an enabled debug exception, it sets the
* low-order bits of this register (B0 thru B3) before entering the debug
* exception handler. Bn is set if the condition described by DRn, LENn, and
* R/Wn occurs. (Note that the processor sets Bn regardless of whether Gn or
* Ln is set. If more than one breakpoint condition occurs at one time and if
* the breakpoint trap occurs due to an enabled condition other than n, Bn may
* be set, even though neither Gn nor Ln is set.)
*
* The BT bit is associated with the T-bit (debug trap bit) of the TSS.
* The processor sets the BT bit before entering the debug handler if a task
* switch has occurred and the T-bit of the new TSS is set. There is no
* corresponding bit in DR7 that enables and disables this trap; the T-bit of
* the TSS is the sole enabling bit.
*
* The BS bit is associated with the TF (trap flag) bit of the EFLAGS
* register. The BS bit is set if the debug handler is entered due to the
* occurrence of a single-step exception. The single-step trap is the
* highest-priority debug exception; therefore, when BS is set, any of the
* other debug status bits may also be set.
*
* The BD bit is set if the next instruction will read or write one of the
* eight debug registers and ICE-386 is also using the debug registers at the
* same time.
*
* Note that the bits of DR6 are never cleared by the processor. To avoid any
* confusion in identifying the next debug exception, the debug handler should
* move zeros to DR6 immediately before returning.
*
* ( INTEL 80386 PROGRAMMER'S REFERENCE MANUAL 1986 )
*/

/* DR6 Bits */
#define X86_DR6_B0       0x00000001
#define X86_DR6_B1       0x00000002
#define X86_DR6_B2       0x00000004
#define X86_DR6_B3       0x00000008
#define X86_DR6_BD       0x00002000
#define X86_DR6_BS       0x00004000
#define X86_DR6_BT       0x00008000

/* DR7 Bits */
#define X86_DR7_L0       0x00000001
#define X86_DR7_G0       0x00000002
#define X86_DR7_L1       0x00000004
#define X86_DR7_G1       0x00000008
#define X86_DR7_L2       0x00000010
#define X86_DR7_G2       0x00000020
#define X86_DR7_L3       0x00000040
#define X86_DR7_G3       0x00000080
#define X86_DR7_LE       0x00000100
#define X86_DR7_GE       0x00000200
#define X86_DR7_GD       0x00002000
#define X86_DR7_W0       0x00010000
#define X86_DR7_R0       0x00020000
#define X86_DR7_LEN0_00  0x00000000
#define X86_DR7_LEN0_01  0x00040000
#define X86_DR7_LEN0_10  0x00080000
#define X86_DR7_LEN0_11  0x000c0000
#define X86_DR7_W1       0x00100000
#define X86_DR7_R1       0x00200000
#define X86_DR7_LEN1_00  0x00000000
#define X86_DR7_LEN1_01  0x00400000
#define X86_DR7_LEN1_10  0x00800000
#define X86_DR7_LEN1_11  0x00c00000
#define X86_DR7_W2       0x01000000
#define X86_DR7_R2       0x02000000
#define X86_DR7_LEN2_00  0x00000000
#define X86_DR7_LEN2_01  0x04000000
#define X86_DR7_LEN2_10  0x08000000
#define X86_DR7_LEN2_11  0x0c000000
#define X86_DR7_W3       0x10000000
#define X86_DR7_R3       0x20000000
#define X86_DR7_LEN3_00  0x00000000
#define X86_DR7_LEN3_01  0x40000000
#define X86_DR7_LEN3_10  0x80000000
#define X86_DR7_LEN3_11  0xc0000000

/*
* Selector format
*
* 15                               3  2  1      0
* -----------------------------------------------
* |    Index                        |TI |  RPL  |
* -----------------------------------------------
* Index - Descriptor index        TI - Descriptor
* RPL   - Requestor's Privelege        table type
*         Level
*/

/* Requestor's Privilege Level */
#define X86_SEL_RPL0  0x0000  /* 0(Supervisor) */
#define X86_SEL_RPL1  0x0001  /* 1(Supervisor) */
#define X86_SEL_RPL2  0x0002  /* 2(Supervisor) */
#define X86_SEL_RPL3  0x0003  /* 3(User)       */

/* Descriptor Table Type */
#define X86_SEL_TI    0x0004  /* Table Indicator (1-local, 0-global) */

/*
* General-Segment Descriptor Format
*
* 31                      16 15                        0
* ------------------------------------------------------
* | Segment Base 15..0      | Segment Limit 15..0      | 0
* ------------------------------------------------------
* |  Base    |G|B|0|A|Limit |P|DP|TY|E|W|A|  Base      | +4
* | 31..24   | |D| |V|19..16| |L |PE|C|R| | 23..16     |
* |          | | | |L|      | |  |  | | | |            |
* ------------------------------------------------------
* G   - Granularity bit          P    - Present bit
* B/D - Data/operation size bit  DPL  - Descriptor Privilege
* AVL - Available for system            Level
*       programmers              TYPE - Descriptor type (data/code)
* E/C - Expand down/Conforming   A    - Accessed bit
*       bit                      W/R  - Writable/Readable bit
*/

/* Access byte */
#define X86_ACB_P         0x00008000  /* Present in memory (1-yes, 0-no)    */
#define X86_ACB_E         0x00000400  /* Expand down (1-down, 0-up)         */
#define X86_ACB_W         0x00000200  /* Writeable (1-yes, 0-no)            */
#define X86_ACB_R         0x00000200  /* Readable (1-yes, 0-no)             */
#define X86_ACB_C         0x00000400  /* Conforming (1-code can be executed */
                                      /* only if CPL>=DPL, 0-code can be    */
                                      /* executed if CPL=DPL)               */
#define X86_ACB_A         0x00000100  /* Accessed (1-yes, 0-no)             */
/* Descriptor type */
#define X86_ACB_TYP_CODE  0x00001800  /* Code descriptor */
#define X86_ACB_TYP_DATA  0x00001000  /* Data descriptor */
/* Descriptor Privilege Levels */
#define X86_ACB_DPL0      0x00000000  /* 0(Supervisor) */
#define X86_ACB_DPL1      0x00002000  /* 1(Supervisor) */
#define X86_ACB_DPL2      0x00004000  /* 2(Supervisor) */
#define X86_ACB_DPL3      0x00006000  /* 3(User)       */

/* Granularity byte */
#define X86_GRB_G         0x00800000  /* Granularity (1-pages, 0-bytes)            */
#define X86_GRB_B         0x00400000  /* Data size (1-32bit, 4Gb, using ESP        */
                                      /*            0-16bit, 64Kb, using SP)       */
#define X86_GRB_D         0x00400000  /* Default operation size (1-32bit, 0-16bit) */
#define X86_GRB_AVL       0x00100000  /* For programmers use                       */


/*
* System Descriptor Format
*
* 31                      16 15                        0
* ------------------------------------------------------
* | Segment Base 15..0      | Segment Limit 15..0      | 0
* ------------------------------------------------------
* |  Base    |G|0|0|0|Limit |P|DP|0|TYPE|    Base      | +4
* | 31..24   | | | | |19..16| |L | |    |   23..16     |
* |          | | | | |      | |  | |    |              |
* ------------------------------------------------------
* G   - Granularity bit         P    - Present bit
* DPL - Descriptor Privilege    TYPE - Descriptor type
*       Level
*/

/* Access byte */
/* Descriptor type */
#define X86_ACB_TYP_AVAILTSS286  0x00000100  /* Available TSS-286      */
#define X86_ACB_TYP_BUSYTSS286   0x00000300  /* Busy TSS-286           */
#define X86_ACB_TYP_LDT          0x00000200  /* Local Descriptor Table */
#define X86_ACB_TYP_AVAILTSS386  0x00000900  /* Available TSS-386+     */
#define X86_ACB_TYP_BUSYTSS386   0x00000b00  /* Busy TSS-386+          */

/*
* Gate Descriptor Format
*
* 31                      16 15                        0
* ------------------------------------------------------
* | Destination Selector    | Destination offset 15..0 | 0
* ------------------------------------------------------
* | Destination offset      |P|DP|0|TYPE|0|0|0|  Word  | +4
* |       31..16            | |L | |    | | | |  Count |
* |                         | |  | |    | | | |  4..0  |
* ------------------------------------------------------
* P    - Present bit         DPL  - Descriptor Privilege
* TYPE - Descriptor type            Level
*/

/* Access byte */
/* Descriptor type */
#define X86_ACB_TYP_CALLGATE286  0x00000400  /* Call Gate 286       */
#define X86_ACB_TYP_TASKGATE286  0x00000500  /* Task Gate 286       */
#define X86_ACB_TYP_INTGATE286   0x00000600  /* Interrupt Gate 286  */
#define X86_ACB_TYP_TRAPGATE286  0x00000700  /* Trap Gate 286       */
#define X86_ACB_TYP_CALLGATE386  0x00000c00  /* Call Gate 386+      */
#define X86_ACB_TYP_TASKGATE386  0x00000d00  /* Task Gate 386+      */
#define X86_ACB_TYP_INTGATE386   0x00000e00  /* Interrupt Gate 386+ */
#define X86_ACB_TYP_TRAPGATE386  0x00000f00  /* Trap Gate 386+      */

/*
* Exception Summary
*
*
* Description               Interrupt   Return Address  Exception     Function That Can Generate      Error Code
*                           Number      Points to       Type          the Exception
*                                       Faulting
*                                       Instruction
*
* Divide error               0          YES             FAULT         DIV, IDIV                       NO
* Debug exceptions           1                                        Any instruction                 NO
* Some debug exceptions are traps and some are faults.  The exception
* handler can determine which has occurred by examining DR6.
* Breakpoint                 3          NO              TRAP          One-byte INT 3                  NO
* Overflow                   4          NO              TRAP          INTO                            NO
* Bounds check               5          YES             FAULT         BOUND                           NO
* Invalid opcode             6          YES             FAULT         Any illegal instruction         NO
* Coprocessor not available  7          YES             FAULT         ESC, WAIT                       NO
* Double fault               8          YES             ABORT         Any instruction that can        YES(Always Zero)
*                                                                     generate an exception
* Coprocessor Segment
* Overrun                    9          NO              ABORT         Any operand of an ESC           YES
*                                                                     instruction that wraps around
*                                                                     the end of a segment.
* Invalid TSS               10          YES             FAULT         JMP, CALL, IRET, any interrupt  YES
* An invalid-TSS fault is not restartable if it occurs during the
* processing of an external interrupt.
* Segment not present       11          YES             FAULT         Any segment-register modifier   YES
* Stack exception           12          YES             FAULT         Any memory reference thru SS    YES
* General Protection        13          YES             FAULT/ABORT   Any memory reference or code    YES
* All GP faults are restartable. If the fault occurs while attempting fetch                           YES
* to vector to the handler for an external interrupt, the interrupted
* program is restartable, but the interrupt may be lost.
*
* Page fault                14          YES             FAULT         Any memory reference or code    YES
*                                                                     fetch
* Coprocessor error         16          YES             FAULT         ESC, WAIT                       NO
* Coprocessor errors are reported as a fault on the first ESC or WAIT
* instruction executed after the ESC instruction that caused the error.
* Align Check (486+)        17          YES             FAULT         Any memory reference            YES(Always Zero)
* Two-byte SW Interrupt     0-255       NO              TRAP          INT n                           NO
*/

/*
* Task State Segment
*
* 31                                16 15                                 0
* -------------------------------------------------------------------------
* |         0000000000000000          |     BACK LINK TO PREVIOUS TSS     |+0
* |                                  ESP0   (stack for CPL 0)             |+4
* |         0000000000000000          |                SS0                |+8
* |                                  ESP1   (stack for CPL 1)             |+C
* |         0000000000000000          |                SS1                |+10
* |                                  ESP2   (stack for CPL 2)             |+14
* |         0000000000000000          |                SS2                |+18
* |                                  CR3   (PDBR)                         |+1C
* |                                  EIP                                  |+20
* |                                 EFLAGS                                |+24
* |                                  EAX                                  |+28
* |                                  ECX                                  |+2C
* |                                  EDX                                  |+30
* |                                  EBX                                  |+34
* |                                  ESP                                  |+38
* |                                  EBP                                  |+3C
* |                                  ESI                                  |+40
* |                                  EDI                                  |+44
* |         0000000000000000          |                ES                 |+48
* |         0000000000000000          |                CS                 |+4C
* |         0000000000000000          |                SS                 |+50
* |         0000000000000000          |                DS                 |+54
* |         0000000000000000          |                FS                 |+58
* |         0000000000000000          |                GS                 |+5C
* |         0000000000000000          |               LDT                 |+60
* |           I/O MAP BASE            |         000000000000000         |T|+64
* -------------------------------------------------------------------------
*
* NOTE: 0 MEANS INTEL RESERVED. DO NOT DEFINE.
*
* T - Debug trap bit.
*
* I/O MAP BASE - refer to Chapter 8 of INTEL 80386 PROGRAMMER'S REFERENCE MANUAL 1986
*
* NOTE: Every TSS must be finished with FFh byte. If you don't use I/O MAP,
*       last byte of I/O MAP BASE must be FFh.
*/

/* TSS Bits */
#define X86_TSS_DBG_T     0x0001
#define X86_TSS_TERM_BYT  0xff     /* Terminator byte (last byte of TSS) */

#endif
