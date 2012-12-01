/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_I386_THREAD_TYPES_H_
#define _PHLOX_ARCH_I386_THREAD_TYPES_H_

/* Stack state for thread */
typedef struct stack_state {
    uint32  esp;  /* stack pointer */
    uint32  ss;   /* stack selector */
} stack_state_t;

/* Architecture dependend thread data */
typedef struct arch_thread {
    stack_state_t  current_stack;                /* Thread stack state */
    uint32         debug_regs[8];                /* Debug registers (dr7 - dr0) */
    bool           fpu_used;                     /* =true if FPU was used */
    uint8          fpu_state[512] _ALIGNED(16);  /* FPU state */
} arch_thread_t;

/* Architecture-dependend process data */
typedef struct arch_process {
    /* nothing for now */
} arch_process_t;

#endif
