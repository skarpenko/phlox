/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_I386_FPU_BITS_H_
#define _PHLOX_ARCH_I386_FPU_BITS_H_

/*
* FPU status register
*
* 15  13    10 9 8 7 6 5 4 3 2 1 0
* ---------------------------------
* |B|C| TOP |C|C|C|E|S|P|U|O|Z|D|I|
* | |3|     |2|1|0|S|F|E|E|E|E|E|E|
* ---------------------------------
* Exception flags             SF  - Stack Fault
* IE - Invalid Operation      ES  - Error Summary Status
* DE - Denormalized Operand   Cx  - Condition Code
* ZE - Zero Divide            TOP - Top of Stack Pointer
* OE - Overflow               B   - FPU Busy
* UE - Underflow
* PE - Precision
*/

/* Exception flags */
#define X86_FPUSR_IE   0x00000001
#define X86_FPUSR_DE   0x00000002
#define X86_FPUSR_ZE   0x00000004
#define X86_FPUSR_OE   0x00000008
#define X86_FPUSR_UE   0x00000010
#define X86_FPUSR_PE   0x00000020

/* Other bits */
#define X86_FPUSR_SF   0x00000040
#define X86_FPUSR_ES   0x00000080
#define X86_FPUSR_C0   0x00000100
#define X86_FPUSR_C1   0x00000200
#define X86_FPUSR_C2   0x00000400
#define X86_FPUSR_TOP  0x00003800
#define X86_FPUSR_C3   0x00004000
#define X86_FPUSR_B    0x00008000

/* Special macro */
#define X86_FPUSR_TOP_GET(a)  ((a & X86_FPUSR_TOP) >> 11)

/*
* FPU control register
*
* 15    12  10 9 8 7 6 5 4 3 2 1 0
* ---------------------------------
* |     |X| R | P |   |P|U|O|Z|D|I|
* |     | | C | C |   |M|M|M|M|M|M|
* ---------------------------------
* Exception masks          PC - Precision Control
* IM - Invalid Operation   RC - Rounding Control
* DM - Denormal Operand    X  - Infinity Control
* ZM - Zero Divide
* OM - Overflow
* UM - Underflow
* PM - Precision
*/

/* Exception masks */
#define X86_FPUCR_IM     0x00000001
#define X86_FPUCR_DM     0x00000002
#define X86_FPUCR_ZM     0x00000004
#define X86_FPUCR_OM     0x00000008
#define X86_FPUCR_UM     0x00000010
#define X86_FPUCR_PM     0x00000020

/* Other bits */
#define X86_FPUCR_PC     0x00000300
#define X86_FPUCR_PC_00  0x00000000  /* Single precision (24 bits)          */
#define X86_FPUCR_PC_01  0x00000100  /* Reserved                            */
#define X86_FPUCR_PC_10  0x00000200  /* Double precision (53 bits)          */
#define X86_FPUCR_PC_11  0x00000300  /* Double Extended precision (64 bits) */
#define X86_FPUCR_RC     0x00000c00
#define X86_FPUCR_RC_00  0x00000000  /* Round to nearest (even)             */
#define X86_FPUCR_RC_01  0x00000400  /* Round down (toward -Inf)            */
#define X86_FPUCR_RC_10  0x00000800  /* Round up (toward +Inf)              */
#define X86_FPUCR_RC_11  0x00000c00  /* Round toward zero (truncate)        */
#define X86_FPUCR_X      0x00001000

/* Special macroses */
#define X86_FPUCR_PC_GET(a)  ((a & X86_FPUCR_PC) >> 8)
#define X86_FPUCR_RC_GET(a)  ((a & X86_FPUCR_RC) >> 10)

/*
* MXCSR control and status register
*
* 31                 15 14 12  10 9 8 7 6 5 4 3 2 1 0
* ----------------------------------------------------
* |                  |F| R |P|U|O|Z|D|I|D|P|U|O|Z|D|I|
* |     Reserved     |Z| C |M|M|M|M|M|M|A|E|E|E|E|E|E|
* |                  | |   | | | | | | |Z| | | | | | |
* ----------------------------------------------------
* IE  - Invalid Operation Flag   IM - Invalid Operation Mask
* DE  - Denormal Flag            DM - Denormal Operation Mask
* ZE  - Divide-by-Zero Flag      ZM - Divide-by-Zero Mask
* OE  - Overflow Flag            OM - Overflow Mask
* UE  - Underflow Flag           UM - Underflow Mask
* PE  - Precision Flag           PM - Precision Mask
* DAZ - Denormals Are Zeros      RC - Rounding Control
*                                FZ - Flush to Zero
*/

#define X86_MXCSR_IE     0x00000001
#define X86_MXCSR_DE     0x00000002
#define X86_MXCSR_ZE     0x00000004
#define X86_MXCSR_OE     0x00000008
#define X86_MXCSR_UE     0x00000010
#define X86_MXCSR_PE     0x00000020
#define X86_MXCSR_DAZ    0x00000040
#define X86_MXCSR_IM     0x00000080
#define X86_MXCSR_DM     0x00000100
#define X86_MXCSR_ZM     0x00000200
#define X86_MXCSR_OM     0x00000400
#define X86_MXCSR_UM     0x00000800
#define X86_MXCSR_PM     0x00001000
#define X86_MXCSR_RC     0x00006000
#define X86_MXCSR_RC_00  0x00000000  /* Round to nearest (even)      */
#define X86_MXCSR_RC_01  0x00002000  /* Round down (toward -Inf)     */
#define X86_MXCSR_RC_10  0x00004000  /* Round up (toward +Inf)       */
#define X86_MXCSR_RC_11  0x00006000  /* Round toward zero (truncate) */
#define X86_MXCSR_FZ     0x00008000

/* Special macro */
#define X86_MXCSR_RC_GET(a)  ((a & X86_MXCSR_RC) >> 13)

#endif
