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
uint32 arch_interrupt_init(kernel_args_t *kargs)
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

/* DEBUG: remove this later*/
   interrupt_enable(0x20); /* enable timer */
   local_irqs_enable();    /* enable interrupts */
/* DEBUG end */

    return 0;
}


/* main interrupt handling routine */
void i386_handle_interrupt(i386_int_frame_t frame); /* lets compiler be happy */
void i386_handle_interrupt(i386_int_frame_t frame)
{
    /* for debugging */
    if(frame.vector < IRQS_BASE_VECTOR) {
        kprint("CPU EXCEPTION OCCURED!!\n");
        kprint(" Stack frame:\n");
        kprint("  gs = 0x%x\n", frame.gs);
        kprint("  fs = 0x%x\n", frame.fs);
        kprint("  es = 0x%x\n", frame.es);
        kprint("  ds = 0x%x\n", frame.ds);
        kprint("  edi = 0x%x\n", frame.edi);
        kprint("  esi = 0x%x\n", frame.esi);
        kprint("  ebp = 0x%x\n", frame.ebp);
        kprint("  esp = 0x%x\n", frame.esp);
        kprint("  ebx = 0x%x\n", frame.ebx);
        kprint("  edx = 0x%x\n", frame.edx);
        kprint("  ecx = 0x%x\n", frame.ecx);
        kprint("  eax = 0x%x\n", frame.eax);
        kprint("  vector = 0x%x (%d)\n", frame.vector, frame.vector);
        kprint("  err_code = 0x%x (%d)\n", frame.err_code, frame.err_code);
        kprint("  eip = 0x%x\n", frame.eip);
        kprint("  cs = 0x%x\n", frame.cs);
        kprint("  eglags = 0x%x\n", frame.eflags);
        kprint("  user_esp = 0x%x\n", frame.user_esp);
        kprint("  user_ss = 0x%x\n", frame.user_ss);

        local_irqs_disable();
        panic(":( :( :(\n");
    }
    if(frame.vector == (uint32)(-1)) {
        local_irqs_disable();
        panic("UNHANDLED INTERRUPT VECTOR\n");
    }

    /* acknowledge hardware interrupt */
    if(frame.vector >= IRQS_BASE_VECTOR &&
       frame.vector <  IRQS_BASE_VECTOR + IRQS_NUMBER)
        interrupt_ack(frame.vector);
}
