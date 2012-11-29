/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <arch/arch_bits.h>
#include <phlox/types.h>
#include <phlox/kargs.h>
#include <phlox/processor.h>

void arch_processor_set_init(arch_processor_set_t *aps, kernel_args_t *kargs, uint32 curr_cpu) {
}

void arch_processor_init(arch_processor_t *ap, kernel_args_t *kargs, uint32 curr_cpu) {
    uint32 cpu;

    /*
     * This asm part returns 3 - for 80386 CPU,
     * 4 - for 80486 CPU without CPUID support
     * and 0 - if CPUID supported and it is possible to
     * obtain extended CPU information.
     * (stolen from Mach)
     */
    asm(
     "   pushfl                      ;"  /* Fetch flags ... */
     "   popl    %%eax               ;"  /*  ... into eax */
     "   movl    %%eax, %%ecx        ;"  /* Save original flags for return */
     "   xorl    $0x00240000, %%eax  ;"  /* Attempt to toggle ID and AC bits */
     "   pushl   %%eax               ;"  /* Save flags... */
     "   popfl                       ;"  /*  ... In EFLAGS */
     "   pushfl                      ;"  /* Fetch flags back ... */
     "   popl    %%eax               ;"  /*  ... into eax */

     "   xorl    %%ecx, %%eax        ;"  /* See if any bits didn't change */
     "   testl   $0x00040000, %%eax  ;"  /* Test AC bit */
     "   jnz 0f                      ;"  /* Skip next bit if AC toggled */
     "   movl    $3, %%eax           ;"  /*   Return value is 386 */
     "   jmp 9f                      ;"  /*   And FINISH */

     "0: testl   $0x00200000, %%eax  ;"  /* Test ID bit */
     "   jnz 0f                      ;"  /* Skip next bit if ID toggled */
     "   movl    $4, %%eax           ;"  /*   Return value is 486 */
     "   jmp 9f                      ;"  /*   And FINISH */

     /* We are a modern enough processor to have the CPUID instruction;
        use it to find out what we are. */
     "0: movl $0, %%eax              ;"  /* Return 0 */
    
     "9: pushl   %%ecx               ;"  /* From ecx... */
     "   popfl                       ;"  /* ... restore original flags */
     : "=a" (cpu) : : "%ecx");

    asm("fninit"); /* init FPU */
}
