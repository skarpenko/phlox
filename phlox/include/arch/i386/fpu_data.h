/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_I386_FPU_DATA_H_
#define _PHLOX_ARCH_I386_FPU_DATA_H_

#include <phlox/compiler.h>
#include <phlox/types.h>

/* fsave/frstor fpu state format */
typedef struct _PACKED {
    uint16 cwd;    /* control word */
    uint16 rsvd1;
    uint16 swd;    /* status word */
    uint16 rsvd2;
    uint16 twd;    /* tag word */
    uint16 rsvd3;
    uint32 fip;    /* instruction pointer offset */
    uint16 fcs;    /* instruction pointer selector */
    uint16 fop;    /* opcode */
    uint32 foo;    /* operand pointer offset */
    uint16 fos;    /* operand pointer selector */
    uint16 rsvd4;
    /* data registers */
    uint8  st0[10];
    uint8  st1[10];
    uint8  st2[10];
    uint8  st3[10];
    uint8  st4[10];
    uint8  st5[10];
    uint8  st6[10];
    uint8  st7[10];
    /* NOTE: rsrvd denotes reserved fields */
} fpu_fsr_state;

/* fxsave/fxrstor fpu state format */
typedef struct _PACKED _ALIGNED(16) {
    uint16 cwd;         /* control word */
    uint16 swd;         /* status word */
    uint8  twd;         /* tag word */
    uint8  rsvd1;
    uint16 fop;         /* opcode */
    uint32 fip;         /* instruction pointer offset */
    uint16 fcs;         /* instruction pointer selector */
    uint16 rsvd2;
    uint32 foo;         /* operand pointer offset */
    uint16 fos;         /* operand pointer selector */
    uint16 rsvd3;
    uint32 mxcsr;       /* mxcsr register state */
    uint32 mxcsr_mask;  /* mxcsr mask */
    /* fpu/mmx data registers */
    uint8  stmm0[10], rsvd4[6];
    uint8  stmm1[10], rsvd5[6];
    uint8  stmm2[10], rsvd6[6];
    uint8  stmm3[10], rsvd7[6];
    uint8  stmm4[10], rsvd8[6];
    uint8  stmm5[10], rsvd9[6];
    uint8  stmm6[10], rsvd10[6];
    uint8  stmm7[10], rsvd11[6];
    /* xmm registers */
    uint8  xmm0[16];
    uint8  xmm1[16];
    uint8  xmm2[16];
    uint8  xmm3[16];
    uint8  xmm4[16];
    uint8  xmm5[16];
    uint8  xmm6[16];
    uint8  xmm7[16];
    uint8  rsvd12[224];
    /* NOTE: rsrvd denotes reserved fields */
} fpu_fxsr_state;

/* fpu state union */
typedef union {
    fpu_fsr_state  fsr;
    fpu_fxsr_state fxsr;
} fpu_state;

#endif
