/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_I386_PROCESSOR_H_
#define _PHLOX_ARCH_I386_PROCESSOR_H_

/*
 * NOTE: Names with prefix "arch_" redefined in upper level processor.h
 * as common for all architectures without this prefix.
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

/*
 * System control registers read/write macroses
 */
#define read_cr0() ({ \
    uint32 __dummy; \
    asm volatile ( \
        "movl %%cr0, %0;" \
        :"=r" (__dummy)); \
    __dummy; \
})

#define write_cr0(x) \
    asm volatile ("movl %0, %%cr0": :"r" (x));

#define read_cr2() ({ \
    uint32 __dummy; \
    asm volatile ( \
        "movl %%cr2, %0;" \
        :"=r" (__dummy)); \
    __dummy; \
})

#define write_cr2(x) \
    asm volatile ("movl %0, %%cr2": :"r" (x));

#define read_cr3() ({ \
    uint32 __dummy; \
    asm volatile ( \
        "movl %%cr3, %0;" \
        :"=r" (__dummy)); \
    __dummy; \
})

#define write_cr3(x) \
    asm volatile ("movl %0, %%cr3": :"r" (x));

#define read_cr4() ({ \
    uint32 __dummy; \
    asm volatile ( \
        "movl %%cr4, %0;" \
        :"=r" (__dummy)); \
    __dummy; \
})

#define write_cr4(x) \
    asm volatile ("movl %0, %%cr4": :"r" (x));


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
    asm volatile ("inb %%dx, %%al" \
    :"=a" (__dummy) \
    :"d" (port)); \
    __dummy; \
})

#define out8(port, value) \
    asm volatile ("outb %%al, %%dx" \
    : :"a" (value), "d" (port))

#define in16(port) ({ \
    uint16 __dummy; \
    asm volatile ("inw %%dx, %%ax" \
    :"=a" (__dummy) \
    :"d" (pot)); \
    __dummy; \
})

#define out16(port, value) \
    asm volatile ("outw %%ax, %%dx" \
    : :"a" (value), "d" (port))

#define in32(port) ({ \
    uint32 __dummy; \
    asm volatile ("inl %%dx, %%eax" \
    :"=a" (__dummy) \
    :"d" (port)); \
    __dummy; \
})

#define out32(port, value) \
    asm volatile ("outl %%eax, %%dx" \
    : :"a" (value), "d" (port))

#define in8_p(port) ({ \
    uint8 __dummy; \
    __dummy = in8(port); \
    asm volatile (__SLOW_DOWN_IO); \
    __dummy; \
})

#define out8_p(port, value) \
    out8(port, value) \
    asm volatile (__SLOW_DOWN_IO)

#define in16_p(port) ({ \
    uint16 __dummy; \
    __dummy = in16(port); \
    asm volatile (__SLOW_DOWN_IO); \
    __dummy; \
})

#define out16_p(port, value) \
    out16(port, value) \
    asm volatile (__SLOW_DOWN_IO)

#define in32_p(port) ({ \
    uint32 __dummy; \
    __dummy = in32(port); \
    asm volatile (__SLOW_DOWN_IO); \
    __dummy; \
})

#define out32_p(port, value) \
    out32(port, value) \
    asm volatile (__SLOW_DOWN_IO)

#define ins8(port, addr, count) \
    asm volatile ("rep; insb" \
    :"+D"(addr), "+c"(count) \
    :"d"(port))

#define outs8(port, addr, count) \
    asm volatile ("rep; outsb" \
    :"+S"(addr), "+c"(count) \
    :"d"(port))

#define ins16(port, addr, count) \
    asm volatile ("rep; insw" \
    :"+D"(addr), "+c"(count) \
    :"d"(port))

#define outs16(port, addr, count) \
    asm volatile ("rep; outsw" \
    :"+S"(addr), "+c"(count) \
    :"d"(port))

#define ins32(port, addr, count) \
    asm volatile ("rep; insl" \
    :"+D"(addr), "+c"(count) \
    :"d"(port))

#define outs32(port, addr, count) \
    asm volatile ("rep; outsl" \
    :"+S"(addr), "+c"(count) \
    :"d"(port))

#define ins8_p(port, addr, count) \
    ins8(port, addr, count); \
    asm volatile (__SLOW_DOWN_IO)

#define outs8_p(port, addr, count) \
    outs8(port, addr, count); \
    asm volatile (__SLOW_DOWN_IO)

#define ins16_p(port, addr, count) \
    ins16(port, addr, count); \
    asm volatile (__SLOW_DOWN_IO)

#define outs16_p(port, addr, count) \
    outs16(port, addr, count); \
    asm volatile (__SLOW_DOWN_IO)

#define ins32_p(port, addr, count) \
    ins32(port, addr, count); \
    asm volatile (__SLOW_DOWN_IO)

#define outs32_p(port, addr, count) \
    outs32(port, addr, count); \
    asm volatile (__SLOW_DOWN_IO)

/*
 * Other i386 architecture specific definitions
 */

/* Flags register store and restore respectively */
#define local_store_flags(x) \
    do { \
      typecheck(unsigned long,x); \
      asm volatile ("pushfl; popl %0;" \
      :"=g" (x) \
      : /* no input */); \
    } while (0)
    
#define local_restore_flags(x)  \
    do { \
      typecheck(unsigned long,x); \
      asm volatile ("pushl %0; popfl;" \
      : /* no output */ \
      :"g" (x) \
      :"memory", "cc"); \
    } while (0)

/*
 * Clear and set Task Switched bit respectively
 */
#define clts()  asm volatile ("clts")
#define stts()  write_cr0(X86_CR0_TS | read_cr0())

/*
 * Write back cache data into memory and invalidate cache
 * Note that WBINVD instruction supported by 486+ CPUs
 */
#if CPU_i386
  #warning WBINVD instruction is not supported on i386 CPU!
  #define wbinvd()
#else
  #define wbinvd()  asm volatile ("wbinvd")
#endif

/*
 * CPU serialization
 * Note that CPUID instruction supported by Pentium+ and some 486 CPUs
 */
#define serialize_cpu() \
    asm volatile ("cpuid" : : : "ax", "bx", "cx", "dx")

/*
 * Invalidate all entries in Translation Lookaside Buffer of MMU
 * by reloading CR3
 */
#define arch_invalidateTLB() \
    write_cr3( read_cr3() )

/*
 * Invalidate single entry in Translation Lookaside Buffer
 * Note that INVLPG instruction supported only by 486+ CPUs
 */
#if CPU_i386
  #warning IVLPG instruction is not supported on i386 CPU!
  /* invalidate all TLB entries */
  #define arch_invalidateTLBentry(virt_addr)  arch_invalidateTLB()
#else
  #define arch_invalidateTLBentry(virt_addr) \
      asm volatile ("invlpg (%0);" : :"r" (virt_addr))
#endif

/* No Operation instruction */
#define arch_nop() asm volatile ("nop")

/* REP NOP (PAUSE) is a good thing to insert into busy-wait loops. */
#define rep_nop()  asm volatile ("rep; nop;")
#define arch_cpu_relax()  rep_nop()

/* interrupt control */
#define arch_local_irqs_enable()   asm volatile ("sti")
#define arch_local_irqs_disable()  asm volatile ("cli")
#define arch_local_irqs_disabled() \
({ \
    uint32 __flags; \
    local_store_flags(__flags); \
    !(__flags & X86_EFLAGS_IF); \
})

/* used in the idle loop; sti takes one instruction cycle to complete */
#define arch_safe_halt()  asm volatile ("sti; hlt;")

/* used when interrupts are already enabled or to shutdown the processor */
#define arch_halt()  asm volatile ("hlt")

#endif
