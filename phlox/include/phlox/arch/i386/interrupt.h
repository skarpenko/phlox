/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_I386_INTERRUPT_H_
#define _PHLOX_ARCH_I386_INTERRUPT_H_

/*
 * Stack frame - this is how stack looks upon
 * entry to kernel from interrupt or user mode
 */
typedef struct _PACKED {
/* 0x00 */  uint32  gs;
/* 0x04 */  uint32  fs;
/* 0x08 */  uint32  es;
/* 0x0C */  uint32  ds;
/* 0x10 */  uint32  edi;
/* 0x14 */  uint32  esi;
/* 0x18 */  uint32  ebp;
/* 0x1C */  uint32  esp;
/* 0x20 */  uint32  ebx;
/* 0x24 */  uint32  edx;
/* 0x28 */  uint32  ecx;
/* 0x2C */  uint32  eax;
/* 0x30 */  uint32  vector;

  /* this is either zero or some error code */
/* 0x34 */  uint32  err_code;

  /* this is extra bonus from Intel */
/* 0x38 */  uint32  eip;
/* 0x3C */  uint32  cs;
/* 0x40 */  uint32  eflags;
/* 0x44 */  uint32  user_esp;
/* 0x48 */  uint32  user_ss;
} i386_int_frame_t;

#endif
