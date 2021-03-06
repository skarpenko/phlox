/*
* Copyright 2007-2012, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_I386_PROCESSOR_H_
#define _PHLOX_ARCH_I386_PROCESSOR_H_

#include <phlox/arch/i386/thread_types.h>


/*
 * NOTE: Names with prefix "arch_" may be redefined in upper level processor.h
 * as common for all architectures without this prefix. Or used by routines
 * of common layer.
 */


/*
 * i386 architecture specific data definitions
 */

/*
 * Size of IO bitmap stored in TSS
 */
#define IO_BITMAP_BITS        65536
#define IO_BITMAP_BYTES       (IO_BITMAP_BITS/8)
#define IO_BITMAP_SHORTS      (IO_BITMAP_BYTES/sizeof(short))
#define IO_BITMAP_LONGS       (IO_BITMAP_BYTES/sizeof(long))
#define IO_BITMAP_LONG_LONGS  (IO_BITMAP_BYTES/sizeof(long long))
#define IO_BITMAP_OFFSET      offsetof(cpu_tss,io_map_base)

/* i386 architecture vendors */
enum i386_vendors {
    I386_VENDOR_UNKNOWN = 0,
    I386_VENDOR_INTEL,      /* Intel Corporation                   */
    I386_VENDOR_AMD,        /* Advanced Micro Devices, Inc.        */
    I386_VENDOR_CYRIX,      /* Cyrix                               */
    I386_VENDOR_CENTAUR,    /* Centaur Technology                  */
    I386_VENDOR_SIS,        /* Silicon Integrated Systems          */
    I386_VENDOR_NEXGEN,     /* NexGen                              */
    I386_VENDOR_TRANSMETA,  /* Transmeta Corporation               */
    I386_VENDOR_RISE,       /* Rise Technology                     */
    I386_VENDOR_UMC,        /* United Microelectronics Corporation */
    I386_VENDOR_NSC,        /* National Semiconductor              */
    I386_VENDOR_VIA,        /* VIA Technologies                    */

    I386_VENDORS_COUNT      /* Total count of known vendors        */
};

/* i386 architecture CPU features */
enum i386_features {
    /* common features */
    I386_FEATURE_C = 0,  /* cpuid eax = 1, ecx register */
    I386_FEATURE_D,      /* cpuid eax = 1, edx register */

    /* AMD processors specific features */
    I386_AMD_FEATURE_C,  /* cpuid eax = 0x80000001, ecx register */
    I386_AMD_FEATURE_D,  /* cpuid eax = 0x80000001, edx register */

    I386_FEATURES_COUNT  /* total features count */
};

/* architectue specific processor set structure */
typedef struct {
    /* architecture specific data for processor set */
} arch_processor_set_t;

/* architectue specific processor structure */
typedef struct {
    enum i386_vendors vendor;  /* processor vendor */
    /* processor features */
    enum i386_features  features[I386_FEATURES_COUNT];
    char vendor_str[13];       /* vendor's string */
    const char *vendor_name;   /* vendor's name */
    char model_name[49];       /* processor's name */
    uint8 family;              /* instruction family */
    uint8 model;               /* model */
    uint8 stepping;            /* stepping id */
    uint8 fpu;                 /* FPU presence */
    uint8 cpuid;               /* CPUID instruction support */
    uint8 rdtsc;               /* RDTSC instruction support */
} arch_processor_t;


/*
 * i386 architecture specific macroses definitions
 */

/*
 * Memory barriers
 */
/* Mandatory barriers */
#define arch_mb()   __asm__ __volatile__ ("lock; addl $0,0(%esp);")  /* memory barrier       */
#define arch_rmb()  __asm__ __volatile__ ("lock; addl $0,0(%esp);")  /* read memory barrier  */
#define arch_wmb()  __asm__ __volatile__ ("lock; addl $0,0(%esp);")  /* write memory barrier */

/* SMP barriers */
#if SYSCFG_SMP_SUPPORT
/* Mandatory barriers for SMP    */
#  define arch_smp_mb()   arch_mb()
#  define arch_smp_rmb()  arch_rmb()
#  define arch_smp_wmb()  arch_wmb()
#else
/* Compiler barriers for non-SMP */
#  define arch_smp_mb()   barrier()
#  define arch_smp_rmb()  barrier()
#  define arch_smp_wmb()  barrier()
#endif

/*
 * System control registers read/write macroses
 */
#define read_cr0() ({ \
    uint32 __dummy; \
    __asm__ __volatile__ ( \
        "movl %%cr0, %0;" \
        :"=r" (__dummy)); \
    __dummy; \
})

#define write_cr0(x) \
    __asm__ __volatile__ ("movl %0, %%cr0": :"r" (x))

#define read_cr2() ({ \
    uint32 __dummy; \
    __asm__ __volatile__ ( \
        "movl %%cr2, %0;" \
        :"=r" (__dummy)); \
    __dummy; \
})

#define write_cr2(x) \
    __asm__ __volatile__ ("movl %0, %%cr2": :"r" (x))

#define read_cr3() ({ \
    uint32 __dummy; \
    __asm__ __volatile__ ( \
        "movl %%cr3, %0;" \
        :"=r" (__dummy)); \
    __dummy; \
})

#define write_cr3(x) \
    __asm__ __volatile__ ("movl %0, %%cr3": :"r" (x))

#define read_cr4() ({ \
    uint32 __dummy; \
    __asm__ __volatile__ ( \
        "movl %%cr4, %0;" \
        :"=r" (__dummy)); \
    __dummy; \
})

#define write_cr4(x) \
    __asm__ __volatile__ ("movl %0, %%cr4": :"r" (x))

/*
 * Hardware debug registers read/write macroses
 */
#define read_dr0() ({ \
    uint32 __dummy; \
    __asm__ __volatile__ ( \
        "movl %%dr0, %0;" \
        :"=r" (__dummy)); \
    __dummy; \
})

#define write_dr0(x) \
    __asm__ __volatile__ ("movl %0, %%dr0": :"r" (x))

#define read_dr1() ({ \
    uint32 __dummy; \
    __asm__ __volatile__ ( \
        "movl %%dr1, %0;" \
        :"=r" (__dummy)); \
    __dummy; \
})

#define write_dr1(x) \
    __asm__ __volatile__ ("movl %0, %%dr1": :"r" (x))

#define read_dr2() ({ \
    uint32 __dummy; \
    __asm__ __volatile__ ( \
        "movl %%dr2, %0;" \
        :"=r" (__dummy)); \
    __dummy; \
})

#define write_dr2(x) \
    __asm__ __volatile__ ("movl %0, %%dr2": :"r" (x))

#define read_dr3() ({ \
    uint32 __dummy; \
    __asm__ __volatile__ ( \
        "movl %%dr3, %0;" \
        :"=r" (__dummy)); \
    __dummy; \
})

#define write_dr3(x) \
    __asm__ __volatile__ ("movl %0, %%dr3": :"r" (x))

/*** According to Intel's manual DR4 and DR5 is reserved. ***/

#define read_dr6() ({ \
    uint32 __dummy; \
    __asm__ __volatile__ ( \
        "movl %%dr6, %0;" \
        :"=r" (__dummy)); \
    __dummy; \
})

#define write_dr6(x) \
    __asm__ __volatile__ ("movl %0, %%dr6": :"r" (x))

#define read_dr7() ({ \
    uint32 __dummy; \
    __asm__ __volatile__ ( \
        "movl %%dr7, %0;" \
        :"=r" (__dummy)); \
    __dummy; \
})

#define write_dr7(x) \
    __asm__ __volatile__ ("movl %0, %%dr7": :"r" (x))


/*
 * I/O macroses
 * in8/out8, in16/out16, in32/out32 - in/out byte, word and double word respectively
 * ins8/outs8, ins16/outs16, ins32/outs32 - in/out sequence of bytes, words and
 * double words respectively.
 * Macroses with suffix "_p" is a paused versions.
 */
#define __SLOW_DOWN_IO "jmp 1f; 1: jmp 1f; 1:" /* slow io by jumping */

#define in8(port) ({ \
    uint8 __dummy; \
    __asm__ __volatile__ ("inb %%dx, %%al" \
    :"=a" (__dummy) \
    :"d" (port)); \
    __dummy; \
})

#define out8(port, value) \
    __asm__ __volatile__ ("outb %%al, %%dx" \
    : :"a" (value), "d" (port))

#define in16(port) ({ \
    uint16 __dummy; \
    __asm__ __volatile__ ("inw %%dx, %%ax" \
    :"=a" (__dummy) \
    :"d" (pot)); \
    __dummy; \
})

#define out16(port, value) \
    __asm__ __volatile__ ("outw %%ax, %%dx" \
    : :"a" (value), "d" (port))

#define in32(port) ({ \
    uint32 __dummy; \
    __asm__ __volatile__ ("inl %%dx, %%eax" \
    :"=a" (__dummy) \
    :"d" (port)); \
    __dummy; \
})

#define out32(port, value) \
    __asm__ __volatile__ ("outl %%eax, %%dx" \
    : :"a" (value), "d" (port))

#define in8_p(port) ({ \
    uint8 __dummy; \
    __dummy = in8(port); \
    __asm__ __volatile__ (__SLOW_DOWN_IO); \
    __dummy; \
})

#define out8_p(port, value) ({ \
    out8(port, value); \
    __asm__ __volatile__ (__SLOW_DOWN_IO); \
})

#define in16_p(port) ({ \
    uint16 __dummy; \
    __dummy = in16(port); \
    __asm__ __volatile__ (__SLOW_DOWN_IO); \
    __dummy; \
})

#define out16_p(port, value) ({ \
    out16(port, value); \
    __asm__ __volatile__ (__SLOW_DOWN_IO); \
})

#define in32_p(port) ({ \
    uint32 __dummy; \
    __dummy = in32(port); \
    __asm__ __volatile__ (__SLOW_DOWN_IO); \
    __dummy; \
})

#define out32_p(port, value) ({ \
    out32(port, value); \
    __asm__ __volatile__ (__SLOW_DOWN_IO); \
})

#define ins8(port, addr, count) \
    __asm__ __volatile__ ("rep; insb" \
    :"+D"(addr), "+c"(count) \
    :"d"(port))

#define outs8(port, addr, count) \
    __asm__ __volatile__ ("rep; outsb" \
    :"+S"(addr), "+c"(count) \
    :"d"(port))

#define ins16(port, addr, count) \
    __asm__ __volatile__ ("rep; insw" \
    :"+D"(addr), "+c"(count) \
    :"d"(port))

#define outs16(port, addr, count) \
    __asm__ __volatile__ ("rep; outsw" \
    :"+S"(addr), "+c"(count) \
    :"d"(port))

#define ins32(port, addr, count) \
    __asm__ __volatile__ ("rep; insl" \
    :"+D"(addr), "+c"(count) \
    :"d"(port))

#define outs32(port, addr, count) \
    __asm__ __volatile__ ("rep; outsl" \
    :"+S"(addr), "+c"(count) \
    :"d"(port))

#define ins8_p(port, addr, count) ({ \
    ins8(port, addr, count); \
    __asm__ __volatile__ (__SLOW_DOWN_IO); \
})

#define outs8_p(port, addr, count) ({ \
    outs8(port, addr, count); \
    __asm__ __volatile__ (__SLOW_DOWN_IO); \
})

#define ins16_p(port, addr, count) ({ \
    ins16(port, addr, count); \
    __asm__ __volatile__ (__SLOW_DOWN_IO); \
})

#define outs16_p(port, addr, count) ({ \
    outs16(port, addr, count); \
    __asm__ __volatile__ (__SLOW_DOWN_IO); \
})

#define ins32_p(port, addr, count) ({ \
    ins32(port, addr, count); \
    __asm__ __volatile__ (__SLOW_DOWN_IO); \
})

#define outs32_p(port, addr, count) {( \
    outs32(port, addr, count); \
    __asm__ __volatile__ (__SLOW_DOWN_IO); \
})

/*
 * Other i386 architecture specific definitions
 */

/* Flags register store and restore respectively */
#define local_store_flags(x) \
    do { \
      typecheck(unsigned long,x); \
      __asm__ __volatile__ ("pushfl; popl %0;" \
      : "=g" (x) \
      : /* no input */); \
    } while (0)
    
#define local_restore_flags(x)  \
    do { \
      typecheck(unsigned long,x); \
      __asm__ __volatile__ ("pushl %0; popfl;" \
      : /* no output */ \
      : "g" (x) \
      : "memory", "cc"); \
    } while (0)

/*
 * Clear and set Task Switched bit respectively
 */
#define clts()  __asm__ __volatile__ ("clts")
#define stts()  write_cr0(X86_CR0_TS | read_cr0())

/*
 * Write back cache data into memory and invalidate cache
 * Note that WBINVD instruction supported by 486+ CPUs
 */
#if CPU_i386
#  warning WBINVD instruction is not supported on i386 CPU!
#  define wbinvd()
#else
#  define wbinvd()  __asm__ __volatile__ ("wbinvd")
#endif

/*
 * CPU serialization
 * Note that CPUID instruction supported by Pentium+ and some 486 CPUs
 */
#define serialize_cpu() \
    __asm__ __volatile__ ("cpuid" : : : "ax", "bx", "cx", "dx")

/*
 * Get current stack pointer
 */
static inline addr_t arch_current_stack_pointer(void)
{
    addr_t __esp;
    __asm__ __volatile__ (
       " movl %%esp, %0; "
       : "=r" (__esp)
    );
    return __esp;
}

/*
 * Invalidate all entries in Translation Lookaside Buffer of MMU
 * by reloading CR3
 */
static inline void arch_invalidate_TLB(void)
{
    __asm__ __volatile__ (
       " movl %%cr3, %%eax; "  /* read CR3 */
       " movl %%eax, %%cr3; "  /* write CR3 */
       : : : "%eax"
    );
}

/*
 * Invalidate single entry in Translation Lookaside Buffer
 * Note: Due to support of INVLPG instruction only by 486+ CPUs
 * different approach used when compatibility with 386 CPUs needed.
 */
#if CPU_i386
void arch_invalidate_TLB_entry(addr_t virt_addr);
#else
static inline void arch_invalidate_TLB_entry(addr_t virt_addr)
{
    __asm__ __volatile__ (
       " invlpg (%0); "
       : :"r" (virt_addr));
}
#endif

/*
 * Invalidate TLB entries refered to given address range
 */
void arch_invalidate_TLB_range(addr_t start, size_t size);

/*
 * Invalidate list of TLB entries
 */
void arch_invalidate_TLB_list(addr_t pages[], size_t count);

/* No Operation instruction */
#define arch_nop() __asm__ __volatile__ ("nop")

/* REP NOP (PAUSE) is a good thing to insert into busy-wait loops. */
#define rep_nop()  __asm__ __volatile__ ("rep; nop;")
#define arch_cpu_relax()  rep_nop()

/* interrupt control */
#define arch_local_irqs_enable()   __asm__ __volatile__ ("sti")
#define arch_local_irqs_disable()  __asm__ __volatile__ ("cli")

/* returns non-zero value if irqs disabled */
#define arch_local_irqs_disabled() \
({ \
    unsigned long __flags; \
    local_store_flags(__flags); \
    !(__flags & X86_EFLAGS_IF); \
})

/* save irqs state and disable them after. used in spinlocks, for example. */
#define arch_local_irqs_save_and_disable(x) \
    do { \
      typecheck(unsigned long,x); \
      __asm__ __volatile__ ("pushfl; cli; popl %0;" \
      : "=g" (x) \
      : /* no input */ \
      : "memory" ); \
      x &= X86_EFLAGS_IF; \
    } while (0)

/* save irqs state */
#define arch_local_irqs_save(x) \
    do { \
      typecheck(unsigned long,x); \
      __asm__ __volatile__ ("pushfl; popl %0;" \
      : "=g" (x) \
      : /* no input */ \
      : "memory" ); \
      x &= X86_EFLAGS_IF; \
    } while (0)

/* restore irqs from previously saved state */
#define arch_local_irqs_restore(x)  \
    do { \
      typecheck(unsigned long,x); \
      __asm__ __volatile__ ( \
         "   cmpl $0, %%eax; " \
         "   je 1f;          " \
         "   sti;            " \
         "   jmp 2f;         " \
         "1: cli;            " \
         "2:                 " \
         : /* no output */ \
         : "a" (x) \
      ); \
    } while (0)

/* used in the idle loop; sti takes one instruction cycle to complete */
#define arch_safe_halt()  __asm__ __volatile__ ("sti; hlt;")

/* used when interrupts are already enabled or to shutdown the processor */
#define arch_halt()  __asm__ __volatile__ ("hlt")

/* used for system hang */
#define arch_hang() __asm__ __volatile__ ("cli; hlt;")


/*
 * i386 architecture specific routines
 */

/*
 * execute CPUID instruction
 * func - function number;
 * eax, ebx, ecx, edx - output variables for registers content;
 */
void i386_cpuid(uint32 func, uint32 *eax, uint32 *ebx, uint32 *ecx, uint32 *edx);
/* execute CPUID and return register data */
uint32 i386_cpuid_eax(uint32 func);
uint32 i386_cpuid_ebx(uint32 func);
uint32 i386_cpuid_ecx(uint32 func);
uint32 i386_cpuid_edx(uint32 func);

/*
 * Build CPU feature string.
 * Maximum possible feature string length defined by I386_CPU_FEATURE_STR_MAX
 */
#define I386_CPU_FEATURE_STR_MAX  400
void i386_cpu_feature_str(arch_processor_t *p, char *str);

/*
 * Read processor's time-stamp counter
 */
uint64 i386_rdtsc();

/*
 * Save, restore and swap FPU context using FSAVE/FRSTOR instructions
 */
void i386_fpu_fsave(fpu_state *state);
void i386_fpu_frstor(fpu_state *state);
void i386_fpu_fsr_swap(fpu_state *old_state, fpu_state *new_state);

/*
 * Save, restore and swap FPU context using FXSAVE/FXRSTOR instructions
 */
void i386_fpu_fxsave(fpu_state *state);
void i386_fpu_fxrstor(fpu_state *state);
void i386_fpu_fxsr_swap(fpu_state *old_state, fpu_state *new_state);

/*
 * Save, restore and swap FPU context using default method detected during
 * checking processor features.
 */
void i386_fpu_context_save(fpu_state *state);
void i386_fpu_context_load(fpu_state *state);
void i386_fpu_context_swap(fpu_state *old_state, fpu_state *new_state);

/*
 * Routines used for context switch
 */
/* context switch */
void i386_context_switch(arch_thread_t *t_from, arch_thread_t *t_to);
/* page directory switch */
void i386_pgdir_switch(addr_t new_pgdir);
/* set kernel stack for next context switch */
void i386_set_kstack(addr_t kstack_top);
/* enter user space via interrupt return simulation */
void i386_enter_uspace(addr_t entry, void *args, addr_t ustack_top, addr_t ret);

/*
 * Routines for hardware debug context save and load.
 * dr7 register is in zero index, dr6 in first and so on.
 */
void i386_dbg_regs_save(uint32 *dbg_regs);
void i386_dbg_regs_load(uint32 *dbg_regs);
void i386_dbg_regs_clear(void);

/*
 * Miscellaneous utility routines
 */

/*
 * Switch stack and call specified function, arg will be passed to called
 * function.
 */
void i386_switch_stack_and_call(addr_t stack, void (*func)(void *), void *arg);


#endif
