/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/arch/i386/segments.h>


/* Common entry for interrupts which provides error code
 */
#define INTERRUPT_ERRC(name, vector) \
.globl name; \
.align 8; \
name: \
    pushl   $vector; \
    jmp     interrupt_start

/* Common entry for interrupts which doesn't provide error code,
 * so.... we just push dummy zero error code
 */
#define INTERRUPT(name, vector) \
.globl name; \
.align 8; \
name: \
    pushl   $0; \
    pushl   $vector; \
    jmp     interrupt_start


/* Functions defined at kernel/syscall.c */
.extern do_sycall_enter  /* void do_syscall_enter(const int syscall) */
.extern do_sycall_leave  /* void do_syscall_leave(void) */


.text

/*
 * Dummy interrupt handler is set for unused gates
 */
.globl dummy_interrupt;
.align 8;
dummy_interrupt:
    pushl   $0;
    pushl   $0xffffffff;     /* vector = -1 */
    jmp     interrupt_start


/**
 ** 0 to 19 - i386 architecture specific exceptions
 **/

/*
 * Exception Name:  Divide Error Exception
 * Exception Class: Fault
 * Error Code:      None
*/
INTERRUPT(interrupt0, 0);

/*
 * Exception Name:  Debug Exception
 * Exception Class: Trap or Fault
 * Error Code:      None
*/
INTERRUPT(interrupt1, 1);

/*
 * Exception Name:  NMI Interrupt
 * Exception Class: Not applicable
 * Error Code:      Not applicable
*/
INTERRUPT(interrupt2, 2);

/*
 * Exception Name:  Breakpoint Exception
 * Exception Class: Trap
 * Error Code:      None
*/
INTERRUPT(interrupt3, 3);

/*
 * Exception Name:  Overflow Exception
 * Exception Class: Trap
 * Error Code:      None
*/
INTERRUPT(interrupt4, 4);

/*
 * Exception Name:  BOUND Range Exceeded Exception
 * Exception Class: Fault
 * Error Code:      None
*/
INTERRUPT(interrupt5, 5);

/*
 * Exception Name:  Invalid Opcode Exception
 * Exception Class: Fault
 * Error Code:      None
*/
INTERRUPT(interrupt6, 6);

/*
 * Exception Name:  Device Not Available Exception
 * Exception Class: Fault
 * Error Code:      None
*/
INTERRUPT(interrupt7, 7);

/*
 * Exception Name:  Double Fault Exception
 * Exception Class: Abort
 * Error Code:      Yes. Always zero.
*/
INTERRUPT_ERRC(interrupt8, 8);

/*
 * Exception Name:  Coprocessor Segment Overrun
 * Exception Class: Abort
 * Error Code:      None
*/
INTERRUPT(interrupt9, 9);

/*
 * Exception Name:  Invalid TSS Exception
 * Exception Class: Fault
 * Error Code:      Yes
*/
INTERRUPT_ERRC(interrupt10, 10);

/*
 * Exception Name:  Segment Not Present
 * Exception Class: Fault
 * Error Code:      Yes
*/
INTERRUPT_ERRC(interrupt11, 11);

/*
 * Exception Name:  Stack Fault Exception
 * Exception Class: Fault
 * Error Code:      Yes
*/
INTERRUPT_ERRC(interrupt12, 12);

/*
 * Exception Name:  General Protection Exception
 * Exception Class: Fault
 * Error Code:      Yes
*/
INTERRUPT_ERRC(interrupt13, 13);

/*
 * Exception Name:  Page-Fault Exception
 * Exception Class: Fault
 * Error Code:      Yes
*/
INTERRUPT_ERRC(interrupt14, 14);

/*
 * VECTOR 15 - RESERVED
 */

/*
 * Exception Name:  x87 FPU Floating-Point Error
 * Exception Class: Fault
 * Error Code:      None. The x87 FPU provides its own error information.
*/
INTERRUPT(interrupt16, 16);

/*
 * Exception Name:  Alignment Check Exception
 * Exception Class: Fault
 * Error Code:      Yes. Always zero.
*/
INTERRUPT_ERRC(interrupt17, 17);

/*
 * Exception Name:  Machine-Check Exception
 * Exception Class: Abort
 * Error Code:      None. Error information provided by machine-check MSRs.
*/
INTERRUPT(interrupt18, 18);

/*
 * Exception Name:  SIMD Floating-Point Exception
 * Exception Class: Fault
 * Error Code:      None
*/
INTERRUPT(interrupt19, 19);

/**
 ** 32 to 255 - User Defined Interrupts
 **/

INTERRUPT(interrupt32, 32);  /* IRQ #0  */
INTERRUPT(interrupt33, 33);  /* IRQ #1  */
INTERRUPT(interrupt34, 34);  /* IRQ #2  */
INTERRUPT(interrupt35, 35);  /* IRQ #3  */
INTERRUPT(interrupt36, 36);  /* IRQ #4  */
INTERRUPT(interrupt37, 37);  /* IRQ #5  */
INTERRUPT(interrupt38, 38);  /* IRQ #6  */
INTERRUPT(interrupt39, 39);  /* IRQ #7  */
INTERRUPT(interrupt40, 40);  /* IRQ #8  */
INTERRUPT(interrupt41, 41);  /* IRQ #9  */
INTERRUPT(interrupt42, 42);  /* IRQ #10 */
INTERRUPT(interrupt43, 43);  /* IRQ #11 */
INTERRUPT(interrupt44, 44);  /* IRQ #12 */
INTERRUPT(interrupt45, 45);  /* IRQ #13 */
INTERRUPT(interrupt46, 46);  /* IRQ #14 */
INTERRUPT(interrupt47, 47);  /* IRQ #15 */

/*
 * Interrupt handling starts here
 */
.align 16
.globl interrupt_start
interrupt_start:
    /* store registers */
    pusha
    pushl   %ds
    pushl   %es
    pushl   %fs
    pushl   %gs
    /* set Kernel's data segment */
    movl    $KERNEL_DATA_SEG, %eax
    cld
    movl    %eax, %ds
    movl    %eax, %es

    /* We store pointer to a stack frame and do not provide it directly as
     * a structure to avoid horrible GCC optimizations.
     */
    pushl   %esp

    /* call to C-function */
    call    i386_handle_interrupt

    /* remove pointer to stack frame */
    addl    $4, %esp

    /* restore registers */
    popl    %gs
    popl    %fs
    popl    %es
    popl    %ds
    popa
    /* no need to restore vector and err_code, so... just add 8 */
    addl    $8, %esp
    /* return from interrupt */
    iret

/*
 * System calls enters here
 *
 * ABI is the following:
 *   eax - system call number;
 *   ebx - arg0;
 *   ecx - arg1;
 *   edx - arg2;
 *   esi - arg3.
 * Return code:
 *   eax - low part;
 *   edx - high part;
 *   ecx - syscall status (0 - success).
 */
.align 16
.globl system_call
system_call:
    /* push error code and vector */
    pushl   $0
    pushl   $0xC0
    /* store registers */
    pusha
    pushl   %ds
    pushl   %es
    pushl   %fs
    pushl   %gs
    /* set Kernel's data segment */
    movl    $KERNEL_DATA_SEG, %edi
    cld
    movl    %edi, %ds
    movl    %edi, %es

    /* check syscall number is correct */
    cmp     nr_syscall_table_entries, %eax
    jge     invalid_syscall

    /* push args for system call */
    pushl   %esi  /* arg3 */
    pushl   %edx  /* arg2 */
    pushl   %ecx  /* arg1 */
    pushl   %ebx  /* arg0 */

    /* do things on enter system call */
    pushl   %eax  /* store syscall number */
    call do_syscall_enter
    popl    %eax  /* restore syscall number */

    /* handle  system call */
    movl    $syscall_table, %ebp
    call    *(%ebp,%eax,4)
    addl    $16, %esp
    /* return value */
    xorl    %ecx, %ecx /* syscall handled */
    movl    %edx, 0x24(%esp)
    movl    %ecx, 0x28(%esp)
    movl    %eax, 0x2c(%esp)

    /* do things on leaving system call */
    call do_syscall_leave

syscall_exit:
    /* restore registers */
    popl    %gs
    popl    %fs
    popl    %es
    popl    %ds
    popa
    /* no need to restore vector and err_code, so... just add 8 */
    addl    $8, %esp
    /* return from interrupt */
    iret
invalid_syscall:
    /* syscall was not handled */
    movl    $-1, %eax
    movl    %eax, 0x24(%esp)
    movl    %eax, 0x28(%esp)
    movl    %eax, 0x2c(%esp)
    jmp syscall_exit
