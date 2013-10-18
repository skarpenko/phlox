/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <arch/cpu.h>
#include <arch/arch_bits.h>
#include <arch/arch_data.h>
#include <phlox/arch/i386/segments.h>
#include <phlox/arch/i386/int_entry.h>
#include <phlox/arch/i386/exceptions.h>
#include <phlox/platform/irq.h>
#include <phlox/processor.h>
#include <phlox/kernel.h>
#include <phlox/vm_private.h>
#include <phlox/scheduler.h>
#include <phlox/thread.h>
#include <phlox/interrupt.h>


/* Interrupt Descriptors Table */
static cpu_gate_desc *idt = NULL;

/* State segment of special task which is called on double fault */
static cpu_tss doublefault_tss;
#define DOUBLEFAULT_STACKSIZE   (1024)
#define DOUBLEFAULT_STACKSTART  (uint32)(doublefault_stack + DOUBLEFAULT_STACKSIZE)
static uint32 doublefault_stack[DOUBLEFAULT_STACKSIZE];

/* =true if previous reschedule operation was skiped. */
static bool resched_pending = false;


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
    i386_set_gate(&idt[n], (addr_t)addr, X86_ACB_TYP_TRAPGATE386, X86_ACB_DPL3);
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
    cpu_seg_desc *gdt;
    cpu_sys_desc *tss_d;

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
    /* Double fault handled in special manner */
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

    /* system call */
    i386_set_syscall_gate(0xc0, &system_call);

    /* Init double fault handler TSS */
    gdt = (cpu_seg_desc *)kargs->arch_args.virt_gdt;
    tss_d = (cpu_sys_desc *)&gdt[DOUBLE_FAULT_TSS >> 3];

    memset(&doublefault_tss, 0, sizeof(cpu_tss));

    /* Init TSS fields */
    doublefault_tss.esp0 = DOUBLEFAULT_STACKSTART;
    doublefault_tss.ss0  = KERNEL_DATA_SEG;
    doublefault_tss.cr3  = read_cr3();
    doublefault_tss.eip  = (uint32)&interrupt8;
    doublefault_tss.esp  = DOUBLEFAULT_STACKSTART;
    doublefault_tss.cs   = KERNEL_CODE_SEG;
    doublefault_tss.ss   = KERNEL_DATA_SEG;
    doublefault_tss.ds   = KERNEL_DATA_SEG;
    doublefault_tss.es   = KERNEL_DATA_SEG;
    doublefault_tss.fs   = KERNEL_DATA_SEG;
    doublefault_tss.ldt  = 0;
    doublefault_tss.eflags = 0x2; /* second bit is always set */
    doublefault_tss.io_map_base = 0x00ff; /* terminator byte */

    /* Init TSS descriptor */
    tss_d->stru.limit_00_15 = sizeof(cpu_tss) & 0xffff;
    tss_d->stru.limit_16_19 = 0;
    tss_d->stru.base_00_15  = (addr_t)&doublefault_tss & 0xffff;
    tss_d->stru.base_16_23  = ((addr_t)&doublefault_tss >> 16) & 0xff;
    tss_d->stru.base_24_31  = (addr_t)&doublefault_tss >> 24;
    tss_d->stru.type        = 0x9; /* TSS descriptor, not busy */
    tss_d->stru.dpl         = 0;
    tss_d->stru.p           = 1;
    tss_d->stru.g           = 1;
    tss_d->stru.zero0       = 0;
    tss_d->stru.zero1       = 0;
    tss_d->stru.zero2       = 0;
    tss_d->stru.zero3       = 0;

    i386_set_task_gate(8, DOUBLE_FAULT_TSS);

    /* completed with no error */
    return 0;
}


/* prints interrupt frame stack */
static void print_int_frame(i386_int_frame_t *frame)
{
    kprint("\nStack frame:\n");
    kprint("Vect = 0x%08x    ErrC = 0x%08x\n", frame->vector, frame->err_code);
    kprint(" EAX = 0x%08x     EBX = 0x%08x\n", frame->eax, frame->ebx);
    kprint(" ECX = 0x%08x     EDX = 0x%08x\n", frame->ecx, frame->edx);
    kprint(" EDI = 0x%08x     ESI = 0x%08x\n", frame->edi, frame->esi);
    kprint(" EBP = 0x%08x     ESP = 0x%08x\n", frame->ebp, frame->esp);
    kprint("  CS = 0x%08x      DS = 0x%08x\n", frame->cs,  frame->ds);
    kprint("  ES = 0x%08x      FS = 0x%08x\n", frame->es,  frame->fs);
    kprint("  GS = 0x%08x      SS = 0x%08x\n", frame->gs,  frame->ds); /* SS = DS */
    kprint(" EIP = 0x%08x  EFLAGS = 0x%08x\n", frame->eip, frame->eflags);
    kprint("\n");
}

/* main interrupt handling routine */
void i386_handle_interrupt(i386_int_frame_t *frame); /* lets compiler be happy */
void i386_handle_interrupt(i386_int_frame_t *frame)
{
    thread_t *th = NULL;
    bool resched_needed = resched_pending; resched_pending = false;

    /* get current thread only if kernel startup stage completed,
     * which means threading is up and running.
     */
    if(is_kernel_ready())
        th = thread_get_current_thread();

    /* if entering exception */
    if(th && frame->vector <= 19)
        ++th->in_exception;

    /* handle vector */
    switch(frame->vector) {
        /* Divide Error Exception */
        case 0:
           kprint("\n\nDivide Error Exception\n");
           print_int_frame(frame);
           panic(":(");
            break;

        /* Debug Exception */
        case 1:
           kprint("\n\nDebug Exception\n");
           print_int_frame(frame);
           panic(":(");
            break;

        /* Nonmaskable Interrupt (NMI) */
        case 2:
           kprint("\n\nNonmaskable Interrupt\n");
           print_int_frame(frame);
           panic(":(");
            break;

        /* Breakpoint Exception */
        case 3:
           kprint("\n\nBreakpoint Exception\n");
           print_int_frame(frame);
           panic(":(");
            break;

        /* Overflow Exception */
        case 4:
           kprint("\n\nOverflow Exception\n");
           print_int_frame(frame);
           panic(":(");
            break;

        /* BOUND Range Exceeded Exception */
        case 5:
           kprint("\n\nBOUND Range Exceeded Exception\n");
           print_int_frame(frame);
           panic(":(");
            break;

        /* Invalid Opcode Exception */
        case 6:
           kprint("\n\nInvalid Opcode Exception\n");
           print_int_frame(frame);
           panic(":(");
            break;

        /* Device Not Available Exception */
        case 7:
           i386_device_not_available();
            break;

        /* Double Fault Exception */
        case 8:
           kprint("\n\nDouble Fault Exception\n");
           panic(":(");
            break;

        /* Coprocessor Segment Overrun */
        case 9:
           kprint("\n\nCoprocessor Segment Overrun\n");
           print_int_frame(frame);
           panic(":(");
            break;

        /* Invalid TSS Exception */
        case 10:
           kprint("\n\nInvalid TSS Exception\n");
           print_int_frame(frame);
           panic(":(");
            break;

        /* Segment Not Present */
        case 11:
           kprint("\n\nSegment Not Present\n");
           print_int_frame(frame);
           panic(":(");
            break;

        /* Stack Fault Exception */
        case 12:
           kprint("\n\nStack Fault Exception\n");
           print_int_frame(frame);
           panic(":(");
            break;

        /* General Protection Exception */
        case 13:
           kprint("\n\nGeneral Protection Exception\n");
           print_int_frame(frame);
           panic(":(");
            break;

        /* Page-Fault Exception */
        case 14: {
            vm_hard_page_fault( read_cr2(), frame->eip,
                                (frame->err_code & 0x02) != 0,   /* is write ? */
                                (frame->err_code & 0x10) != 0,   /* is exec ?  */
                                (frame->err_code & 0x04) != 0 ); /* is user ?  */
        }
         break;

        /** Vector 15 is reserved on IA-32, so...
         ** if occurs it will be handled as unhandled.
         **/

        /* x87 FPU Floating-Point Error */
        case 16:
           kprint("\n\nx87 FPU Floating-Point Error\n");
           print_int_frame(frame);
           panic(":(");
            break;

        /* Alignment Check Exception */
        case 17:
           kprint("\n\nAlignment Check Exception\n");
           print_int_frame(frame);
           panic(":(");
            break;

        /* Machine-Check Exception */
        case 18:
           kprint("\n\nMachine-Check Exception\n");
           print_int_frame(frame);
           panic(":(");
            break;

        /* SIMD Floating-Point Exception */
        case 19:
           kprint("\n\nSIMD Floating-Point Exception\n");
           print_int_frame(frame);
           panic(":(");
            break;

        /* Unhandled vector */
        case 0xffffffff:
           kprint("\n\nUnhandled interrupt vector\n");
           print_int_frame(frame);
           panic(":(");
            break;

        /* Here starts handling of hardware interrupts */
        default: {
            flags_t res;

            /* ensure that hardware interrupt occured */
            if(frame->vector >= IRQS_BASE_VECTOR &&
               frame->vector <  IRQS_BASE_VECTOR + IRQS_NUMBER) {
                /* entering hardware interrupt */
                if(th) ++th->in_interrupt;
                /* acknowledge hardware interrupt */
                interrupt_ack(frame->vector);
                /* start interrupt handling */
                res = handle_hw_interrupt(frame->vector-IRQS_BASE_VECTOR);
                /* check result */
                if(res & INT_FLAGS_RESCHED)
                    resched_needed = true;
                /* leaving hardware interrupt */
                if(th) --th->in_interrupt;
            }
        }
         break;
    }

    /* if leaving exception */
    if(th && frame->vector <= 19)
        --th->in_exception;

    /* reschedule if needed */
    if(resched_needed) {
        if(sched_lock_tryacquire())
            sched_reschedule();
        else
            resched_pending = true;
    }
}
