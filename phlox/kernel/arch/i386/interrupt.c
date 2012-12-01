/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <arch/cpu.h>
#include <arch/arch_bits.h>
#include <arch/arch_data.h>
#include <phlox/arch/i386/segments.h>
#include <phlox/arch/i386/int_entry.h>
#include <phlox/platform/irq.h>
#include <phlox/processor.h>
#include <phlox/kernel.h>
#include <phlox/vm_private.h>
#include <phlox/interrupt.h>


/* Interrupt Descriptors Table */
cpu_gate_desc *idt = NULL;


/*
 * Set of locally used routines
 */

/* Sets gate descriptor with given routine address, type and DPL */
static void i386_set_gate(cpu_gate_desc *gate_ptr, addr_t addr, uint32 type, uint32 dpl)
{
    uint32 tmp_dword0, tmp_dword1; /* temporary dwords */

    /* preparing gate data. do not do this in place! */
    tmp_dword0 = (KERNEL_CODE_SEG << 16) | (0x0000ffff & addr);
    tmp_dword1 = (0xffff0000 & addr) | X86_ACB_P | dpl | type;

    /* setting gate */
    gate_ptr->raw.dword0 = tmp_dword0;
    gate_ptr->raw.dword1 = tmp_dword1;
}

/* Sets given gate as system call gate */
static void i386_set_syscall_gate(uint32 n, void *addr)
{
    i386_set_gate(&idt[n], (addr_t)addr, X86_ACB_TYP_CALLGATE386, X86_ACB_DPL3);
}

/* Sets given gate as interrupt gate */
static void i386_set_intr_gate(uint32 n, void *addr)
{
    i386_set_gate(&idt[n], (addr_t)addr, X86_ACB_TYP_INTGATE386, X86_ACB_DPL0);
}

/* Sets given gate as trap gate */
static void i386_set_trap_gate(uint32 n, void *addr)
{
    i386_set_gate(&idt[n], (addr_t)addr, X86_ACB_TYP_TRAPGATE386, X86_ACB_DPL0);
}

/* Sets given gate as task gate */
static void i386_set_task_gate(uint32 n, uint16 tss_seg)
{
    uint32 tmp_dword0, tmp_dword1; /* temporary dwords */

    /* prepare */
    tmp_dword0 = (tss_seg << 16);
    tmp_dword1 = X86_ACB_P | X86_ACB_DPL0 | X86_ACB_TYP_TASKGATE;

    /* set */
    idt[n].raw.dword0 = tmp_dword0;
    idt[n].raw.dword1 = tmp_dword1;
}


/* init interrupt handling */
status_t arch_interrupt_init(kernel_args_t *kargs)
{
    uint32 i;

    /* init local idt pointer */
    idt = (cpu_gate_desc *)kargs->arch_args.virt_idt;

    /* pre-init gates */
    for(i=0; i < (MAX_IDT_LIMIT/sizeof(cpu_gate_desc)); i++)
       i386_set_intr_gate(i, &dummy_interrupt);

    /* set interrupt gates */
    i386_set_intr_gate(0,   &interrupt0);
    i386_set_intr_gate(1,   &interrupt1);
    i386_set_intr_gate(2,   &interrupt2);
    i386_set_intr_gate(3,   &interrupt3);
    i386_set_intr_gate(4,   &interrupt4);
    i386_set_intr_gate(5,   &interrupt5);
    i386_set_intr_gate(6,   &interrupt6);
    i386_set_intr_gate(7,   &interrupt7);
    i386_set_intr_gate(8,   &interrupt8);
    i386_set_intr_gate(9,   &interrupt9);
    /**/
    i386_set_intr_gate(10,  &interrupt10);
    i386_set_intr_gate(11,  &interrupt11);
    i386_set_intr_gate(12,  &interrupt12);
    i386_set_intr_gate(13,  &interrupt13);
    i386_set_intr_gate(14,  &interrupt14);
    i386_set_intr_gate(16,  &interrupt16);
    i386_set_intr_gate(17,  &interrupt17);
    i386_set_intr_gate(18,  &interrupt18);
    i386_set_intr_gate(19,  &interrupt19);
    /**/
    i386_set_intr_gate(32,  &interrupt32);
    i386_set_intr_gate(33,  &interrupt33);
    i386_set_intr_gate(34,  &interrupt34);
    i386_set_intr_gate(35,  &interrupt35);
    i386_set_intr_gate(36,  &interrupt36);
    i386_set_intr_gate(37,  &interrupt37);
    i386_set_intr_gate(38,  &interrupt38);
    i386_set_intr_gate(39,  &interrupt39);
    /**/
    i386_set_intr_gate(40,  &interrupt40);
    i386_set_intr_gate(41,  &interrupt41);
    i386_set_intr_gate(42,  &interrupt42);
    i386_set_intr_gate(43,  &interrupt43);
    i386_set_intr_gate(44,  &interrupt44);
    i386_set_intr_gate(45,  &interrupt45);
    i386_set_intr_gate(46,  &interrupt46);
    i386_set_intr_gate(47,  &interrupt47);

    /* completed with no error */
    return 0;
}


/* main interrupt handling routine */
void i386_handle_interrupt(i386_int_frame_t frame); /* lets compiler be happy */
void i386_handle_interrupt(i386_int_frame_t frame)
{
    switch(frame.vector) {
        /* Divide Error Exception */
        case 0:
         break;

        /* Debug Exception */
        case 1:
         break;

        /* Nonmaskable Interrupt (NMI) */
        case 2:
         break;

        /* Breakpoint Exception */
        case 3:
         break;

        /* Overflow Exception */
        case 4:
         break;

        /* BOUND Range Exceeded Exception */
        case 5:
         break;

        /* Invalid Opcode Exception */
        case 6:
         break;

        /* Device Not Available Exception */
        case 7:
         break;

        /* Double Fault Exception */
        case 8:
         break;

        /* Coprocessor Segment Overrun */
        case 9:
         break;

        /* Invalid TSS Exception */
        case 10:
         break;

        /* Segment Not Present */
        case 11:
         break;

        /* Stack Fault Exception */
        case 12:
         break;

        /* General Protection Exception */
        case 13:
         break;

        /* Page-Fault Exception */
        case 14: {
            vm_hard_page_fault( read_cr2(), frame.eip,
                                (frame.err_code & 0x02) != 0,   /* is write ? */
                                (frame.err_code & 0x10) != 0,   /* is exec ?  */
                                (frame.err_code & 0x04) != 0 ); /* is user ?  */
        }
         break;

        /** Vector 15 is reserved on IA-32, so...
         ** if occurs it will be handled as unhandled.
         **/

        /* x87 FPU Floating-Point Error */
        case 16:
         break;

        /* Alignment Check Exception */
        case 17:
         break;

        /* Machine-Check Exception */
        case 18:
         break;

        /* SIMD Floating-Point Exception */
        case 19:
         break;

        /* Unhandled vector */
        case 0xffffffff:
         break;

        /* Here starts handling of hardware interrupts */
        default: {
            /* ensure that hardware interrupt occured */
            if(frame.vector >= IRQS_BASE_VECTOR &&
               frame.vector <  IRQS_BASE_VECTOR + IRQS_NUMBER) {
                /* acknowledge hardware interrupt */
                interrupt_ack(frame.vector);
                /* start interrupt handling */
                handle_hw_interrupt(frame.vector-IRQS_BASE_VECTOR);
            }
        }
         break;
    }
}
