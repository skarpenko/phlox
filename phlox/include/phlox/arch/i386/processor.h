/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_I386_PROCESSOR_H_
#define _PHLOX_ARCH_I386_PROCESSOR_H_

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
    uint8 cpuid;               /* CPUID instruction support */
    uint8 rdtsc;               /* RDTSC instruction support */
} arch_processor_t;


/*
 * i386 architecture specific macroses definitions
 */

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
    __asm__ __volatile__ ("movl %0, %%cr0": :"r" (x));

#define read_cr2() ({ \
    uint32 __dummy; \
    __asm__ __volatile__ ( \
        "movl %%cr2, %0;" \
        :"=r" (__dummy)); \
    __dummy; \
})

#define write_cr2(x) \
    __asm__ __volatile__ ("movl %0, %%cr2": :"r" (x));

#define read_cr3() ({ \
    uint32 __dummy; \
    __asm__ __volatile__ ( \
        "movl %%cr3, %0;" \
        :"=r" (__dummy)); \
    __dummy; \
})

#define write_cr3(x) \
    __asm__ __volatile__ ("movl %0, %%cr3": :"r" (x));

#define read_cr4() ({ \
    uint32 __dummy; \
    __asm__ __volatile__ ( \
        "movl %%cr4, %0;" \
        :"=r" (__dummy)); \
    __dummy; \
})

#define write_cr4(x) \
    __asm__ __volatile__ ("movl %0, %%cr4": :"r" (x));


/*
 * I/O macroses
 * in8/out8, in16/out16, in32/out32 - in/out byte, word and double word respectively
 * ins8/outs8, ins16/outs16, ins32/outs32 - in/out sequence of bytes, words and
 * double words respectively.
 * Macroses with suffix "_p" is a paused versions.
 */
#define __SLOW_DOWN_IO "jmp 1f; 1: jmp 1f 1:" /* slow io by jumping */

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

#define out8_p(port, value) \
    out8(port, value) \
    __asm__ __volatile__ (__SLOW_DOWN_IO)

#define in16_p(port) ({ \
    uint16 __dummy; \
    __dummy = in16(port); \
    __asm__ __volatile__ (__SLOW_DOWN_IO); \
    __dummy; \
})

#define out16_p(port, value) \
    out16(port, value) \
    __asm__ __volatile__ (__SLOW_DOWN_IO)

#define in32_p(port) ({ \
    uint32 __dummy; \
    __dummy = in32(port); \
    __asm__ __volatile__ (__SLOW_DOWN_IO); \
    __dummy; \
})

#define out32_p(port, value) \
    out32(port, value) \
    __asm__ __volatile__ (__SLOW_DOWN_IO)

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

#define ins8_p(port, addr, count) \
    ins8(port, addr, count); \
    __asm__ __volatile__ (__SLOW_DOWN_IO)

#define outs8_p(port, addr, count) \
    outs8(port, addr, count); \
    __asm__ __volatile__ (__SLOW_DOWN_IO)

#define ins16_p(port, addr, count) \
    ins16(port, addr, count); \
    __asm__ __volatile__ (__SLOW_DOWN_IO)

#define outs16_p(port, addr, count) \
    outs16(port, addr, count); \
    __asm__ __volatile__ (__SLOW_DOWN_IO)

#define ins32_p(port, addr, count) \
    ins32(port, addr, count); \
    __asm__ __volatile__ (__SLOW_DOWN_IO)

#define outs32_p(port, addr, count) \
    outs32(port, addr, count); \
    __asm__ __volatile__ (__SLOW_DOWN_IO)

/*
 * Other i386 architecture specific definitions
 */

/* Flags register store and restore respectively */
#define local_store_flags(x) \
    do { \
      typecheck(unsigned long,x); \
      __asm__ __volatile__ ("pushfl; popl %0;" \
      :"=g" (x) \
      : /* no input */); \
    } while (0)
    
#define local_restore_flags(x)  \
    do { \
      typecheck(unsigned long,x); \
      __asm__ __volatile__ ("pushl %0; popfl;" \
      : /* no output */ \
      :"g" (x) \
      :"memory", "cc"); \
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
  #warning WBINVD instruction is not supported on i386 CPU!
  #define wbinvd()
#else
  #define wbinvd()  __asm__ __volatile__ ("wbinvd")
#endif

/*
 * CPU serialization
 * Note that CPUID instruction supported by Pentium+ and some 486 CPUs
 */
#define serialize_cpu() \
    __asm__ __volatile__ ("cpuid" : : : "ax", "bx", "cx", "dx")

/*
 * Invalidate all entries in Translation Lookaside Buffer of MMU
 * by reloading CR3
 */
static inline void arch_invalidate_TLB() {
    __asm__ __volatile__ (
       " movl %cr3, %eax; "  /* read CR3 */
       " movl %eax, %cr3; "  /* write CR3 */
       : : : "%eax"
    );
}

/*
 * Invalidate single entry in Translation Lookaside Buffer
 * Note that INVLPG instruction supported only by 486+ CPUs
 */
#if CPU_i386
  #warning IVLPG instruction is not supported on i386 CPU!
  /* invalidate all TLB entries */
  #define arch_invalidate_TLB_entry(virt_addr)  arch_invalidate_TLB()
#else
static inline void invalidate_TLB_entry(addr_t virt_addr) {
    __asm__ __volatile__ (
       " invlpg (%0); "
       : :"r" (virt_addr));
}
#endif

/* No Operation instruction */
#define arch_nop() __asm__ __volatile__ ("nop")

/* REP NOP (PAUSE) is a good thing to insert into busy-wait loops. */
#define rep_nop()  __asm__ __volatile__ ("rep; nop;")
#define arch_cpu_relax()  rep_nop()

/* interrupt control */
#define arch_local_irqs_enable()   __asm__ __volatile__ ("sti")
#define arch_local_irqs_disable()  __asm__ __volatile__ ("cli")
#define arch_local_irqs_disabled() \
({ \
    uint32 __flags; \
    local_store_flags(__flags); \
    !(__flags & X86_EFLAGS_IF); \
})

/* used in the idle loop; sti takes one instruction cycle to complete */
#define arch_safe_halt()  __asm__ __volatile__ ("sti; hlt;")

/* used when interrupts are already enabled or to shutdown the processor */
#define arch_halt()  __asm__ __volatile__ ("hlt")


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

#endif
