/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_I386_CPU_DATA_H_
#define _PHLOX_ARCH_I386_CPU_DATA_H_

#include <phlox/compiler.h>
#include <phlox/types.h>

/*
* Address pair (far pointer format)
*
* 47       32 31                      0
* -------------------------------------
* | Selector |         Offset         |
* -------------------------------------
*/
typedef struct _PACKED {
    uint32 offset;
    uint16 selector;
} cpu_addr_pair;


/*
* Descriptor table pointer
*
* 47                    16 15         0
* -------------------------------------
* |    Linear address     |   Limit   |
* -------------------------------------
*/
typedef struct _PACKED {
    uint16 limit;
    uint32 address;
} cpu_desc_tbl_ptr;


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
typedef struct _PACKED {
    uint16 rpl:    2;
    uint16 ti:     1;
    uint16 index: 13;
} cpu_selector_struc;

/* raw format */
typedef struct _PACKED {
    uint16 word0;
} cpu_selector_raw;

/* union */
typedef union {
    cpu_selector_struc stru;
    cpu_selector_raw   raw;
} cpu_selector;


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
typedef struct _PACKED {
    uint16 limit_00_15;
    uint16 base_00_15;
    uint32 base_16_23:  8;
    /* access byte      */
    uint32 a:           1;
    uint32 wr:          1;
    uint32 ec:          1;
    uint32 type:        2;
    uint32 dpl:         2;
    uint32 p:           1;
    /* granularity byte */
    uint32 limit_16_19: 4;
    uint32 avl:         1;
    uint32 zero0:       1;
    uint32 bd:          1;
    uint32 g:           1;
    /********************/
    uint32 base_24_31:  8;
} cpu_seg_desc_struc;

/* raw format */
typedef struct _PACKED {
    uint32 dword0;
    uint32 dword1;
} cpu_seg_desc_raw;

/* union */
typedef union {
    cpu_seg_desc_struc stru;
    cpu_seg_desc_raw   raw;
} cpu_seg_desc;


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
typedef struct _PACKED {
    uint16 limit_00_15;
    uint16 base_00_15;
    uint32 base_16_23:  8;
    /* access byte      */
    uint32 type:        4;
    uint32 zero0:       1;
    uint32 dpl:         2;
    uint32 p:           1;
    /* granularity byte */
    uint32 limit_16_19: 4;
    uint32 zero1:       1;
    uint32 zero2:       1;
    uint32 zero3:       1;
    uint32 g:           1;
    /********************/
    uint32 base_24_31:  8;
} cpu_sys_desc_struc;

/* raw format */
typedef struct _PACKED {
    uint32 dword0;
    uint32 dword1;
} cpu_sys_desc_raw;

/* union */
typedef union {
    cpu_sys_desc_struc stru;
    cpu_sys_desc_raw   raw;
} cpu_sys_desc;


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
typedef struct _PACKED {
    uint16 offset_00_15;
    uint16 selector;
    uint16 wcount: 5;
    uint16 zero0:  1;
    uint16 zero1:  1;
    uint16 zero2:  1;
    /* access byte */
    uint16 type:   4;
    uint16 zero3:  1;
    uint16 dpl:    2;
    uint16 p:      1;
    /***************/
    uint16 offset_16_31;
} cpu_gate_desc_struc;

/* raw format */
typedef struct _PACKED {
    uint32 dword0;
    uint32 dword1;
} cpu_gate_desc_raw;

/* union */
typedef union {
    cpu_gate_desc_struc stru;
    cpu_gate_desc_raw   raw;
} cpu_gate_desc;


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
typedef struct _PACKED {
    uint16 back_link;
    uint16 unused0;      /* always zero */
    uint32 esp0;
    uint16 ss0;
    uint16 unused1;      /* always zero */
    uint32 esp1;
    uint16 ss1;
    uint16 unused2;      /* always zero */
    uint32 esp2;
    uint16 ss2;
    uint16 unused3;      /* always zero */
    uint32 cr3;
    uint32 eip;
    uint32 eflags;
    uint32 eax;
    uint32 ecx;
    uint32 edx;
    uint32 ebx;
    uint32 esp;
    uint32 ebp;
    uint32 esi;
    uint32 edi;
    uint16 es;
    uint16 unused4;      /* always zero */
    uint16 cs;
    uint16 unused5;      /* always zero */
    uint16 ss;
    uint16 unused6;      /* always zero */
    uint16 ds;
    uint16 unused7;      /* always zero */
    uint16 fs;
    uint16 unused8;      /* always zero */
    uint16 gs;
    uint16 unused9;      /* always zero */
    uint16 ldt;
    uint16 unused10;     /* always zero */
    uint16 t: 1;
    uint16 unused11: 15; /* always zero */
    uint16 io_map_base;
} cpu_tss;

#endif
