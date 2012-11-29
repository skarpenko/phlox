; 32 Bit Data Structures Header
; Version 1.3
; Copyright(C) Carp co., 2001
; Copyright(C) Stepan Karpenko, 2001
;
; History:
; 15-02-01 Added System Descriptor structure and its bits.
;          Added Gate Descriptor structure and its bits.
;          Fixed some comments.
; 16-02-01 Added selector bits.
; 18-02-01 Fixed some comments.
; 19-02-01 Added control registers descriptions and control flags.
;          Added paging structures.
;          Added exception summary.
; 20-02-01 Added Page Directory Entry for 4Mb pages.
; 22-02-01 Fixed some comments.
; 23-03-01 Yeah, The MIR is dead :(
;          Added EFLAGS register description and flags.
;          Added Task State Segment structure, its bits and description.
; 26-03-01 Added debug registers descriptions and its bits.
;          Added PAE structures.
;          Fixed some comments.
;


; Control registers and flags
;
; CR0
; 31       18  16    5 4 3 2 1 0
; -------------------------------
; |P|C|N|  |A| |W|  |N|E|T|E|M|P|
; |G|D|W|..|M| |P|..|E|T|S|M|P|E|
; | | | |  | | | |  | | | | | | |
; -------------------------------
; CR1
; 31                           0
; -------------------------------
; |                             |
; |                             |
; |                             |
; -------------------------------
; CR2
; 31                           0
; -------------------------------
; |                             |
; | PAGE FAULT LINEAR ADDRESS   |
; |                             |
; -------------------------------
; CR3
; 31             12    4 3     0
; -------------------------------
; |      PAGE     |   |P|P| | | |
; |    DIRECTORY  |...|C|W| | | |
; |      BASE     |   |D|T| | | |
; -------------------------------
; CR4
; 31           8 7 6 5 4 3 2 1 0
; -------------------------------
; |           |P|P|M|P|P|D|T|P|V|
; |...........|C|G|C|A|S|E|S|V|M|
; |           |E|E|E|E|E| |D|I|E|
; -------------------------------
; PE - Protection Enable              PCD - Page-Level Cache Disable (486+)
; MP - Monitor Processor Extension    PWT - Page-Level Writes Throught (486+)
; EM - Processor Extension Emulated   VME - Virtual-8086 Mode Extensions
; TS - Task Switch                    PVI - Protected-Mode Virtual Interrupts
; ET - Extension Type (486+)          TSD - Time Stamp Disable
; NE - Numeric Error (486+)           DE  - Debugging Extensions
; WP - Write Protect                  PSE - Page Size Extension
; AM - Alignment Mask                 PAE - Physical Address Extension
; NW - Not Writethrought              MCE - Machine-Check Enable (P5+)
; CD - Cache Disable                  PGE - Paging Global Extensions (P6+)
; PG - Paging Enable                  PCE - Performance-monitoring
;                                           Counter Enable
;

; CR0 Bits
BIT_CR0_PE EQU 00000000000000000000000000000001b
BIT_CR0_MP EQU 00000000000000000000000000000010b
BIT_CR0_EM EQU 00000000000000000000000000000100b
BIT_CR0_TS EQU 00000000000000000000000000001000b
BIT_CR0_ET EQU 00000000000000000000000000010000b
BIT_CR0_NE EQU 00000000000000000000000000100000b
BIT_CR0_WP EQU 00000000000000010000000000000000b
BIT_CR0_AM EQU 00000000000001000000000000000000b
BIT_CR0_NW EQU 00100000000000000000000000000000b
BIT_CR0_CD EQU 01000000000000000000000000000000b
BIT_CR0_PG EQU 10000000000000000000000000000000b
; CR3 Bits
BIT_CR3_PWT EQU 00000000000000000000000000001000b
BIT_CR3_PCD EQU 00000000000000000000000000010000b
; CR4 Bits
BIT_CR4_VME EQU 00000000000000000000000000000001b
BIT_CR4_PVI EQU 00000000000000000000000000000010b
BIT_CR4_TSD EQU 00000000000000000000000000000100b
BIT_CR4_DE  EQU 00000000000000000000000000001000b
BIT_CR4_PSE EQU 00000000000000000000000000010000b
BIT_CR4_PAE EQU 00000000000000000000000000100000b
BIT_CR4_MCE EQU 00000000000000000000000001000000b
BIT_CR4_PGE EQU 00000000000000000000000010000000b
BIT_CR4_PCE EQU 00000000000000000000000100000000b

; EFLAGS Register
;
; 31      21          15             8               0
; -----------------------------------------------------
; |       | |V|V| | | | | | | | | | | | | | | | | | | |
; |   0   |I|I|I|A|V|R|0|N|IO |O|D|I|T|S|Z|0|A|0|P|1|C|
; |       |D|P|F|C|M|F| |T| PL|F|F|F|F|F|F| |F| |F| |F|                       
; |       |x|x|x|x|x|x| |x|x|x|s|c|x|x|s|s| |s| |s| |s|
; -----------------------------------------------------
;
; s - STATUS FLAG, c - CONTROL FLAG, x - SYSTEM FLAG
;
; ID   - ID Flag                         DF - Direction Flag
; VIP  - Virtual Interrupt Pending       IF - Interrupt-enable Flag
; VIF  - Virtual Interrupt Flag          TF - Trap Flag
; AC   - Alignment Check                 SF - Sign Flag
; VM   - Virtual 8086 Mode               ZF - Zero Flag
; RF   - Resume Flag                     AF - Auxiliary Flag
; IOPL - Input/Output Privilege Level    PF - Parity Flag
; NT   - Nested Task Flag                CF - Carry Flag
; OF   - Overflow Flag
;

; EFLAGS Bits
BIT_EFLAGS_CF     EQU 00000000000000000000000000000001b
BIT_EFLAGS_PF     EQU 00000000000000000000000000000100b
BIT_EFLAGS_AF     EQU 00000000000000000000000000010000b
BIT_EFLAGS_ZF     EQU 00000000000000000000000001000000b
BIT_EFLAGS_SF     EQU 00000000000000000000000010000000b
BIT_EFLAGS_TF     EQU 00000000000000000000000100000000b
BIT_EFLAGS_IF     EQU 00000000000000000000001000000000b
BIT_EFLAGS_DF     EQU 00000000000000000000010000000000b
BIT_EFLAGS_OF     EQU 00000000000000000000100000000000b
BIT_EFLAGS_IOPL_0 EQU 00000000000000000000000000000000b
BIT_EFLAGS_IOPL_1 EQU 00000000000000000001000000000000b
BIT_EFLAGS_IOPL_2 EQU 00000000000000000010000000000000b
BIT_EFLAGS_IOPL_3 EQU 00000000000000000011000000000000b
BIT_EFLAGS_NT     EQU 00000000000000000100000000000000b
BIT_EFLAGS_RF     EQU 00000000000000010000000000000000b
BIT_EFLAGS_VM     EQU 00000000000000100000000000000000b
BIT_EFLAGS_AC     EQU 00000000000001000000000000000000b
BIT_EFLAGS_VIF    EQU 00000000000010000000000000000000b
BIT_EFLAGS_VIP    EQU 00000000000100000000000000000000b
BIT_EFLAGS_ID     EQU 00000000001000000000000000000000b

; FLAGS Bits
BIT_FLAGS_CF     EQU 0000000000000001b
BIT_FLAGS_PF     EQU 0000000000000100b
BIT_FLAGS_AF     EQU 0000000000010000b
BIT_FLAGS_ZF     EQU 0000000001000000b
BIT_FLAGS_SF     EQU 0000000010000000b
BIT_FLAGS_TF     EQU 0000000100000000b
BIT_FLAGS_IF     EQU 0000001000000000b
BIT_FLAGS_DF     EQU 0000010000000000b
BIT_FLAGS_OF     EQU 0000100000000000b
BIT_FLAGS_IOPL_0 EQU 0000000000000000b
BIT_FLAGS_IOPL_1 EQU 0001000000000000b
BIT_FLAGS_IOPL_2 EQU 0010000000000000b
BIT_FLAGS_IOPL_3 EQU 0011000000000000b
BIT_FLAGS_NT     EQU 0100000000000000b

; Debug registers and flags
;
; DR0
; 31                                                              0
; -----------------------------------------------------------------
; |                BREAKPOINT 0 LINEAR ADDRESS                    |
; |                                                               |
; -----------------------------------------------------------------
; DR1
; 31                                                              0
; -----------------------------------------------------------------
; |                BREAKPOINT 1 LINEAR ADDRESS                    |
; |                                                               |
; -----------------------------------------------------------------
; DR2
; 31                                                              0
; -----------------------------------------------------------------
; |                BREAKPOINT 2 LINEAR ADDRESS                    |
; |                                                               |
; -----------------------------------------------------------------
; DR3
; 31                                                              0
; -----------------------------------------------------------------
; |                BREAKPOINT 3 LINEAR ADDRESS                    |
; |                                                               |
; -----------------------------------------------------------------
; DR4
; 31                                                              0
; -----------------------------------------------------------------
; |                          RESERVED                             |
; |                                                               |
; -----------------------------------------------------------------
; DR5
; 31                                                              0
; -----------------------------------------------------------------
; |                          RESERVED                             |
; |                                                               |
; -----------------------------------------------------------------
; DR6
; 31              23              15               7              0
; -----------------------------------------------------------------
; |0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|B|B|B|0|0|0|0|0|0|0|0|0|B|B|B|B|
; |1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|T|S|D| |1|1|1|1|1|1|1|1|3|2|1|0|
; -----------------------------------------------------------------
; DR7
; 31              23              15               7              0
; -----------------------------------------------------------------
; |LEN|R|W|LEN|R|W|LEN|R|W|LEN|R|W|0|0|G|0|0|0|G|L|G|L|G|L|G|L|G|L|
; | 3 |3|3| 2 |2|2| 1 |1|1| 0 |0|0| | |D| | |1|E|E|3|3|2|2|1|1|0|0|
; -----------------------------------------------------------------
;
; 0 - 386, 486
; 1- P5+
;
; GD - Global Debug Register Access Detect.
;      (If GD=1 INT 1 occurs when debug registers are accessed.)
;
; Debug Control Register (DR7)
; 
; The debug control register both helps to define the debug conditions
; and selectively enables and disables those conditions.
; 
; For each address in registers DR0-DR3, the corresponding fields R/W0
; through R/W3 specify the type of action that should cause a breakpoint. The
; processor interprets these bits as follows:
; 
;    00 - Break on instruction execution only
;    01 - Break on data writes only
;    10 - undefined
;    11 - Break on data reads or writes but not instruction fetches
; 
; Fields LEN0 through LEN3 specify the length of data item to be monitored. A
; length of 1, 2, or 4 bytes may be specified. The values of the length fields
; are interpreted as follows:
; 
;    00 - one-byte length
;    01 - two-byte length
;    10 - undefined
;    11 - four-byte length
; 
; If RWn is 00 (instruction execution), then LENn should also be 00. Any other
; length is undefined.
; 
; The low-order eight bits of DR7 (L0 through L3 and G0 through G3)
; selectively enable the four address breakpoint conditions. There are two
; levels of enabling: the local (L0 through L3) and global (G0 through G3)
; levels. The local enable bits are automatically reset by the processor at
; every task switch to avoid unwanted breakpoint conditions in the new task.
; The global enable bits are not reset by a task switch; therefore, they can
; be used for conditions that are global to all tasks.
; 
; The LE and GE bits control the "exact data breakpoint match" feature of the
; processor. If either LE or GE is set, the processor slows execution so that
; data breakpoints are reported on the instruction that causes them. It is
; recommended that one of these bits be set whenever data breakpoints are
; armed. The processor clears LE at a task switch but does not clear GE.
; 
; 
;
; Debug Status Register (DR6)
; 
; The debug status register permits the debugger to determine which debug
; conditions have occurred.
; 
; When the processor detects an enabled debug exception, it sets the
; low-order bits of this register (B0 thru B3) before entering the debug
; exception handler. Bn is set if the condition described by DRn, LENn, and
; R/Wn occurs. (Note that the processor sets Bn regardless of whether Gn or
; Ln is set. If more than one breakpoint condition occurs at one time and if
; the breakpoint trap occurs due to an enabled condition other than n, Bn may
; be set, even though neither Gn nor Ln is set.)
; 
; The BT bit is associated with the T-bit (debug trap bit) of the TSS.
; The processor sets the BT bit before entering the debug handler if a task
; switch has occurred and the T-bit of the new TSS is set. There is no
; corresponding bit in DR7 that enables and disables this trap; the T-bit of
; the TSS is the sole enabling bit.
; 
; The BS bit is associated with the TF (trap flag) bit of the EFLAGS
; register. The BS bit is set if the debug handler is entered due to the
; occurrence of a single-step exception. The single-step trap is the
; highest-priority debug exception; therefore, when BS is set, any of the
; other debug status bits may also be set.
; 
; The BD bit is set if the next instruction will read or write one of the
; eight debug registers and ICE-386 is also using the debug registers at the
; same time.
; 
; Note that the bits of DR6 are never cleared by the processor. To avoid any
; confusion in identifying the next debug exception, the debug handler should
; move zeros to DR6 immediately before returning.
;
; ( INTEL 80386 PROGRAMMER'S REFERENCE MANUAL 1986 )
;

; DR6 Bits
BIT_DR6_B0 EQU 00000000000000000000000000000001b
BIT_DR6_B1 EQU 00000000000000000000000000000010b
BIT_DR6_B2 EQU 00000000000000000000000000000100b
BIT_DR6_B3 EQU 00000000000000000000000000001000b
BIT_DR6_BD EQU 00000000000000000010000000000000b
BIT_DR6_BS EQU 00000000000000000100000000000000b
BIT_DR6_BT EQU 00000000000000001000000000000000b

; DR7 Bits
BIT_DR7_L0      EQU 00000000000000000000000000000001b
BIT_DR7_G0      EQU 00000000000000000000000000000010b
BIT_DR7_L1      EQU 00000000000000000000000000000100b
BIT_DR7_G1      EQU 00000000000000000000000000001000b
BIT_DR7_L2      EQU 00000000000000000000000000010000b
BIT_DR7_G2      EQU 00000000000000000000000000100000b
BIT_DR7_L3      EQU 00000000000000000000000001000000b
BIT_DR7_G3      EQU 00000000000000000000000010000000b
BIT_DR7_LE      EQU 00000000000000000000000100000000b
BIT_DR7_GE      EQU 00000000000000000000001000000000b
BIT_DR7_GD      EQU 00000000000000000010000000000000b
BIT_DR7_W0      EQU 00000000000000010000000000000000b
BIT_DR7_R0      EQU 00000000000000100000000000000000b
BIT_DR7_LEN0_00 EQU 00000000000000000000000000000000b
BIT_DR7_LEN0_01 EQU 00000000000001000000000000000000b
BIT_DR7_LEN0_10 EQU 00000000000010000000000000000000b
BIT_DR7_LEN0_11 EQU 00000000000011000000000000000000b
BIT_DR7_W1      EQU 00000000000100000000000000000000b
BIT_DR7_R1      EQU 00000000001000000000000000000000b
BIT_DR7_LEN1_00 EQU 00000000000000000000000000000000b
BIT_DR7_LEN1_01 EQU 00000000010000000000000000000000b
BIT_DR7_LEN1_10 EQU 00000000100000000000000000000000b
BIT_DR7_LEN1_11 EQU 00000000110000000000000000000000b
BIT_DR7_W2      EQU 00000001000000000000000000000000b
BIT_DR7_R2      EQU 00000010000000000000000000000000b
BIT_DR7_LEN2_00 EQU 00000000000000000000000000000000b
BIT_DR7_LEN2_01 EQU 00000100000000000000000000000000b
BIT_DR7_LEN2_10 EQU 00001000000000000000000000000000b
BIT_DR7_LEN2_11 EQU 00001100000000000000000000000000b
BIT_DR7_W3      EQU 00010000000000000000000000000000b
BIT_DR7_R3      EQU 00100000000000000000000000000000b
BIT_DR7_LEN3_00 EQU 00000000000000000000000000000000b
BIT_DR7_LEN3_01 EQU 01000000000000000000000000000000b
BIT_DR7_LEN3_10 EQU 10000000000000000000000000000000b
BIT_DR7_LEN3_11 EQU 11000000000000000000000000000000b

; Selector format
;
; 15                               3  2  1      0
; -----------------------------------------------
; |    Index                        |TI |  RPL  |
; -----------------------------------------------
; Index - Descriptor index        TI - Descriptor
; RPL   - Requestor's Privelege        table type
;         Level
;

; Requestor's Privilege Level
BIT_SEL_RPL0 EQU 0000000000000000b  ; 0(Supervisor)
BIT_SEL_RPL1 EQU 0000000000000001b  ; 1(Supervisor)
BIT_SEL_RPL2 EQU 0000000000000010b  ; 2(Supervisor)
BIT_SEL_RPL3 EQU 0000000000000011b  ; 3(User)
; Descriptor Table Type
BIT_SEL_TI   EQU 0000000000000100b  ; Table Indicator (1-local, 0-global)


; General-Segment Descriptor Format
;
; 31                      16 15                        0
; ------------------------------------------------------
; | Segment Base 15..0      | Segment Limit 15..0      | 0
; ------------------------------------------------------
; |  Base    |G|B|0|A|Limit |P|DP|TY|E|W|A|  Base      | +4
; | 31..24   | |D| |V|19..16| |L |PE|C|R| | 23..16     |
; |          | | | |L|      | |  |  | | | |            |
; ------------------------------------------------------
; G   - Granularity bit          P    - Present bit
; B/D - Data/operation size bit  DPL  - Descriptor Privilege
; AVL - Available for system            Level
;       programmers              TYPE - Descriptor type (data/code)
; E/C - Expand down/Conforming   A    - Accessed bit
;       bit                      W/R  - Writable/Readable bit
;                                 

g_descriptor   STRUC
    limit_0_15  DW 0  ; limit bits (0..15)
    base_0_15   DW 0  ; base bits (0..15)
    base_16_23  DB 0  ; base bits (16..23)
    access      DB 0  ; access byte
    gran        DB 0  ; granularity byte
    base_24_31  DB 0  ; base bits (24..31)
g_descriptor   ENDS

; Granularity byte
BIT_GRB_G   EQU 10000000b   ; Granularity (1-pages, 0-bytes)
BIT_GRB_B   EQU 01000000b   ; Data size (1-32bit, 4Gb, using ESP
                            ;            0-16bit, 64Kb, using SP)
BIT_GRB_D   EQU 01000000b   ; Default operation size (1-32bit, 0-16bit)
BIT_GRB_AVL EQU 00010000b   ; For programmers use

; Access byte
BIT_ACB_P EQU 10000000b     ; Present in memory (1-yes, 0-no)
BIT_ACB_E EQU 00000100b     ; Expand down (1-down, 0-up)
BIT_ACB_W EQU 00000010b     ; Writeable (1-yes, 0-no)
BIT_ACB_R EQU 00000010b     ; Readable (1-yes, 0-no)
BIT_ACB_C EQU 00000100b     ; Conforming (1-code can be executed
                            ;   only if CPL>=DPL, 0-code can be
                            ;   executed if CPL=DPL)
BIT_ACB_A EQU 00000001b     ; Accessed (1-yes, 0-no)
; Descriptor type
BIT_ACB_TYP_CODE EQU 00011000b  ; Code descriptor
BIT_ACB_TYP_DATA EQU 00010000b  ; Data descriptor
; Descriptor Privilege Levels
BIT_ACB_DPL0 EQU 00000000b   ; 0(Supervisor)
BIT_ACB_DPL1 EQU 00100000b   ; 1(Supervisor)
BIT_ACB_DPL2 EQU 01000000b   ; 2(Supervisor)
BIT_ACB_DPL3 EQU 01100000b   ; 3(User)


; System Descriptor Format
;
; 31                      16 15                        0
; ------------------------------------------------------
; | Segment Base 15..0      | Segment Limit 15..0      | 0
; ------------------------------------------------------
; |  Base    |G|0|0|0|Limit |P|DP|0|TYPE|    Base      | +4
; | 31..24   | | | | |19..16| |L | |    |   23..16     |
; |          | | | | |      | |  | |    |              |
; ------------------------------------------------------
; G   - Granularity bit         P    - Present bit
; DPL - Descriptor Privilege    TYPE - Descriptor type
;       Level
;

s_descriptor   STRUC
    limit_0_15  DW 0  ; limit bits (0..15)
    base_0_15   DW 0  ; base bits (0..15)
    base_16_23  DB 0  ; base bits (16..23)
    access      DB 0  ; access byte
    gran        DB 0  ; granularity byte
    base_24_31  DB 0  ; base bits (24..31)
s_descriptor   ENDS

; Access byte
; Descriptor type
BIT_ACB_TYP_AVAILTSS286 EQU 00000001b   ; Available TSS-286
BIT_ACB_TYP_BUSYTSS286  EQU 00000011b   ; Busy TSS-286
BIT_ACB_TYP_LDT         EQU 00000010b   ; Local Descriptor Table
BIT_ACB_TYP_AVAILTSS386 EQU 00001001b   ; Available TSS-386+
BIT_ACB_TYP_BUSYTSS386  EQU 00001011b   ; Busy TSS-386+


; Gate Descriptor Format
;
; 31                      16 15                        0
; ------------------------------------------------------
; | Destination Selector    | Destination offset 15..0 | 0
; ------------------------------------------------------
; | Destination offset      |P|DP|0|TYPE|0|0|0|  Word  | +4
; |       31..16            | |L | |    | | | |  Count |
; |                         | |  | |    | | | |  4..0  |
; ------------------------------------------------------
; P    - Present bit         DPL  - Descriptor Privilege
; TYPE - Descriptor type            Level
;        

i_descriptor   STRUC
    offs_0_15   DW 0  ; destination offset (0..15)
    selector    DW 0  ; destination selector
    words_0_4   DB 0  ; word count (0..4)
    access      DB 0  ; access byte
    offs_16_31  DW 0  ; destination offset (16..31)
i_descriptor   ENDS

; Access byte
; Descriptor type
BIT_ACB_TYP_CALLGATE286 EQU 00000100b    ; Call Gate 286
BIT_ACB_TYP_TASKGATE286 EQU 00000101b    ; Task Gate 286
BIT_ACB_TYP_INTGATE286  EQU 00000110b    ; Interrupt Gate 286
BIT_ACB_TYP_TRAPGATE286 EQU 00000111b    ; Trap Gate 286
BIT_ACB_TYP_CALLGATE386 EQU 00001100b    ; Call Gate 386+
BIT_ACB_TYP_TASKGATE386 EQU 00001101b    ; Task Gate 386+
BIT_ACB_TYP_INTGATE386  EQU 00001110b    ; Interrupt Gate 386+
BIT_ACB_TYP_TRAPGATE386 EQU 00001111b    ; Trap Gate 386+

; Exception Summary
;
;
; Description               Interrupt   Return Address  Exception     Function That Can Generate      Error Code
;                           Number      Points to       Type          the Exception
;                                       Faulting
;                                       Instruction
;
; Divide error               0          YES             FAULT         DIV, IDIV                       NO
; Debug exceptions           1                                        Any instruction                 NO
; Some debug exceptions are traps and some are faults.  The exception
; handler can determine which has occurred by examining DR6.
; Breakpoint                 3          NO              TRAP          One-byte INT 3                  NO
; Overflow                   4          NO              TRAP          INTO                            NO
; Bounds check               5          YES             FAULT         BOUND                           NO
; Invalid opcode             6          YES             FAULT         Any illegal instruction         NO
; Coprocessor not available  7          YES             FAULT         ESC, WAIT                       NO
; Double fault               8          YES             ABORT         Any instruction that can        YES(Always Zero)
;                                                                     generate an exception
; Coprocessor Segment
; Overrun                    9          NO              ABORT         Any operand of an ESC           YES
;                                                                     instruction that wraps around
;                                                                     the end of a segment.
; Invalid TSS               10          YES             FAULT         JMP, CALL, IRET, any interrupt  YES
; An invalid-TSS fault is not restartable if it occurs during the
; processing of an external interrupt.
; Segment not present       11          YES             FAULT         Any segment-register modifier   YES
; Stack exception           12          YES             FAULT         Any memory reference thru SS    YES
; General Protection        13          YES             FAULT/ABORT   Any memory reference or code    YES
; All GP faults are restartable. If the fault occurs while attempting fetch                           YES
; to vector to the handler for an external interrupt, the interrupted
; program is restartable, but the interrupt may be lost.
;                                                                     
; Page fault                14          YES             FAULT         Any memory reference or code    YES
;                                                                     fetch
; Coprocessor error         16          YES             FAULT         ESC, WAIT                       NO
; Coprocessor errors are reported as a fault on the first ESC or WAIT
; instruction executed after the ESC instruction that caused the error.
; Align Check (486+)        17          YES             FAULT         Any memory reference            YES(Always Zero)
; Two-byte SW Interrupt     0-255       NO              TRAP          INT n                           NO
;


; Page Directory entry format
;
; 31                12 11    9 8 7 6 5 4 3 2 1 0
; -----------------------------------------------
; |                   | Avail-|G|P|0|A|P|P|U|R|P|
; |PAGE TABLE ADDRESS | able  | |S| | |C|W|/|/| |
; |                   |       | | | | |D|T|S|W| |
; -----------------------------------------------
; P   - Present                    PCD - Page Cache Disable (486+)
; R/W - Readable/Writable (0/1)    A   - Accessed
; U/S - User/Supervisor (1/0)      PS  - Page Size (P5+)
; PWT - Page Write Throught (486+) G   - Global (P6+)
;

pg_pde   STRUC
    attrib     db 0  ; attributes (0..7)
    avail      db 0  ; (0) - G, (1..3) - available, (4..7) - base (0..4)
    base_5_19  dw 0  ; base (5..19)
pg_pde   ENDS


; Page Table entry format
;
; 31                12 11    9 8 7 6 5 4 3 2 1 0
; -----------------------------------------------
; |                   | Avail-|G|0|D|A|P|P|U|R|P|
; |PAGE FRAME ADDRESS | able  | | | | |C|W|/|/| |
; |                   |       | | | | |D|T|S|W| |
; -----------------------------------------------
; P   - Present                    PCD - Page Cache Disable (486+)
; R/W - Readable/Writable (0/1)    A   - Accessed
; U/S - User/Supervisor (1/0)      D   - Dirty
; PWT - Page Write Throught (486+) G   - Global (P6+)
;

pg_pte   STRUC
    attrib     db 0  ; attributes (0..7)
    avail      db 0  ; (0) - G, (1..3) - available, (4..7) - base (0..4)
    base_5_19  dw 0  ; base (5..19)
pg_pte   ENDS

; Attributes byte
BIT_PG_ATB_P   EQU 00000001b  ; Present
BIT_PG_ATB_W   EQU 00000010b  ; Writable
BIT_PG_ATB_R   EQU 00000000b  ; Read Only
BIT_PG_ATB_U   EQU 00000100b  ; User
BIT_PG_ATB_S   EQU 00000000b  ; Supervisor
BIT_PG_ATB_PWT EQU 00001000b  ; Page Write Throught (486+)
BIT_PG_ATB_PCD EQU 00010000b  ; Page Cache Disable (486+)
BIT_PG_ATB_A   EQU 00100000b  ; Accessed
BIT_PG_ATB_D   EQU 01000000b  ; Dirty
BIT_PG_ATB_PS  EQU 10000000b  ; Page Size (P5+)
; Avail byte
BIT_PG_AVB_G   EQU 00000001b  ; Global (P6+)


; Page Directory entry format for 4Mb pages (P5+)
;
; 31       22 21    12 11    9 8 7 6 5 4 3 2 1 0
; -----------------------------------------------
; |PAGE FRAME|        | Avail-|G|P|D|A|P|P|U|R|P|
; | ADDRESS  |Reserved| able  | |S| | |C|W|/|/| |
; |          |        |       | | | | |D|T|S|W| |
; -----------------------------------------------
; P   - Present                    A   - Accessed
; R/W - Readable/Writable (0/1)    D   - Dirty
; U/S - User/Supervisor (1/0)      PS  - Page Size (P5+)
; PWT - Page Write Throught (486+) G   - Global (P6+)
; PCD - Page Cache Disable (486+)
; 

pg_pde_pse   STRUC
    attrib     db 0  ; attributes (0..7)
    avail      db 0  ; (0) - G, (1..3) - available, (4..7) - reserved
    base_0_9   dw 0  ; (0..5) - reserved, (6..15) - base (0..9)
pg_pde_pse   ENDS


; Task State Segment
;
; 31                                16 15                                 0
; -------------------------------------------------------------------------
; | 000000000000000000000000000000000 |     BACK LINK TO PREVIOUS TSS     |+0
; |                                  ESP0   (stack for CPL 0)             |+4
; | 000000000000000000000000000000000 |                SS0                |+8
; |                                  ESP1   (stack for CPL 1)             |+C
; | 000000000000000000000000000000000 |                SS1                |+10
; |                                  ESP2   (stack for CPL 2)             |+14
; | 000000000000000000000000000000000 |                SS2                |+18
; |                                  CR3   (PDBR)                         |+1C
; |                                  EIP                                  |+20
; |                                 EFLAGS                                |+24
; |                                  EAX                                  |+28
; |                                  ECX                                  |+2C
; |                                  EDX                                  |+30
; |                                  EBX                                  |+34
; |                                  ESP                                  |+38
; |                                  EBP                                  |+3C
; |                                  ESI                                  |+40
; |                                  EDI                                  |+44
; | 000000000000000000000000000000000 |                ES                 |+48
; | 000000000000000000000000000000000 |                CS                 |+4C
; | 000000000000000000000000000000000 |                SS                 |+50
; | 000000000000000000000000000000000 |                DS                 |+54
; | 000000000000000000000000000000000 |                FS                 |+58
; | 000000000000000000000000000000000 |                GS                 |+5C
; | 000000000000000000000000000000000 |               LDT                 |+60
; |           I/O MAP BASE            | 0000000000000000000000000000000 |T|+64
; -------------------------------------------------------------------------
;
; NOTE: 0 MEANS INTEL RESERVED. DO NOT DEFINE.
;
; T - Debug trap bit.
;
; I/O MAP BASE - refer to Chapter 8 of INTEL 80386 PROGRAMMER'S REFERENCE MANUAL 1986
;
; NOTE: Every TSS must be finished with FFh byte. If you don't use I/O MAP,
;       last byte of I/O MAP BASE must be FFh.
;

task_state_segment   STRUC
  back_link   dw 0
              dw 0
  r_esp0      dd 0
  r_ss0       dw 0
              dw 0
  r_esp1      dd 0
  r_ss1       dw 0
              dw 0
  r_esp2      dd 0
  r_ss2       dw 0
              dw 0
  r_cr3       dd 0
  r_eip       dd 0
  r_eflags    dd 0
  r_eax       dd 0
  r_ecx       dd 0
  r_edx       dd 0
  r_ebx       dd 0
  r_esp       dd 0
  r_ebp       dd 0
  r_esi       dd 0
  r_edi       dd 0
  r_es        dw 0
              dw 0
  r_cs        dw 0
              dw 0
  r_ss        dw 0
              dw 0
  r_ds        dw 0
              dw 0
  r_fs        dw 0
              dw 0
  r_gs        dw 0
              dw 0
  ldt         dw 0
              dw 0
  dbg         dw 0
  io_map_base dw 0
task_state_segment   ENDS

; TSS Bits
BIT_TSS_DBG_T EQU 0000000000000001b


; Paging with Physical Address Extension (P6+)
;
; Page Directory Pointer Table Entry
;
; 31          12 11  9 8               0
; ---------------------------------------
; | Base address|     | | | | |P|P| | | |
; | of page dir.|AVAIL|  res  |C|W|res|1| +0
; |  (0..19)    |     | | | | |D|T| | | |
; ---------------------------------------
; |                              | ...  |
; |     RESERVED ALWAYS 0        | (20..| +4
; |                              |  23) |
; ---------------------------------------
; 63                           36 35   32
;
; AVAIL - Available
; res   - Reserved
;

pg_pdpte   STRUC
    attrib     db 0  ; attributes (0..7)
    avail      db 0  ; (0) - G, (1..3) - available, (4..7) - base (0..4)
    base_5_19  dw 0  ; base (5..19)
    base_20_23 dw 0  ; base (20..23), reserved (0..11)
    reserved   dw 0  ; reserved bits (12..31)
pg_pdpte   ENDS


; Page Directory Entry for 4K Pages
;
; 31          12 11  9 8               0
; ---------------------------------------
; | Base address|     | | | | |P|P|U|R| |
; | of page tab.|AVAIL|0|0|0|A|C|W|/|/|P| +0
; |  (0..19)    |     | | | | |D|T|S|W| |
; ---------------------------------------
; |                              | ...  |
; |     RESERVED ALWAYS 0        | (20..| +4
; |                              |  23) |
; ---------------------------------------
; 63                           36 35   32
;
; AVAIL - Available
;

pg_pde4k   STRUC
    attrib     db 0  ; attributes (0..7)
    avail      db 0  ; (0) - G, (1..3) - available, (4..7) - base (0..4)
    base_5_19  dw 0  ; base (5..19)
    base_20_23 dw 0  ; base (20..23), reserved (0..11)
    reserved   dw 0  ; reserved bits (12..31)
pg_pde4k   ENDS


; Page Table Entry for 4K Pages
;
; 31          12 11  9 8               0
; ---------------------------------------
; | Base address|     | | | | |P|P|U|R| |
; |   of page   |AVAIL|G|0|D|A|C|W|/|/|P| +0
; |  (0..19)    |     | | | | |D|T|S|W| |
; ---------------------------------------
; |                              | ...  |
; |     RESERVED ALWAYS 0        | (20..| +4
; |                              |  23) |
; ---------------------------------------
; 63                           36 35   32
;
; AVAIL - Available
;

pg_pte4k   STRUC
    attrib     db 0  ; attributes (0..7)
    avail      db 0  ; (0) - G, (1..3) - available, (4..7) - base (0..4)
    base_5_19  dw 0  ; base (5..19)
    base_20_23 dw 0  ; base (20..23), reserved (0..11)
    reserved   dw 0  ; reserved bits (12..31)
pg_pte4k   ENDS


; Page Directory Entry for 2M Pages
;
; 31          12 11  9 8               0
; ---------------------------------------
; | Base address|     | | | | |P|P|U|R| |
; |   of page   |AVAIL|G|1|D|A|C|W|/|/|P| +0
; |  (0..19)    |     | | | | |D|T|S|W| |
; ---------------------------------------
; |                              | ...  |
; |     RESERVED ALWAYS 0        | (20..| +4
; |                              |  23) |
; ---------------------------------------
; 63                           36 35   32
;
; AVAIL - Available
;

pg_pde2m   STRUC
    attrib     db 0  ; attributes (0..7)
    avail      db 0  ; (0) - G, (1..3) - available, (4..7) - base (0..4)
    base_5_19  dw 0  ; base (5..19)
    base_20_23 dw 0  ; base (20..23), reserved (0..11)
    reserved   dw 0  ; reserved bits (12..31)
pg_pde2m   ENDS

;END
