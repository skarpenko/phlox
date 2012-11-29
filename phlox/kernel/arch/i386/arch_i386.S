/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/

#define FUNCTION(x) .global x; .type x,@function; x
#define GLOBAL(x) .global x; x

/* Arguments */
#define ARG0   4(%esp)
#define ARG1   8(%esp)
#define ARG2  12(%esp)
#define ARG3  16(%esp)
#define ARG4  20(%esp)
#define ARG5  24(%esp)
#define ARG6  28(%esp)

.text

/* void i386_cpuid(uint32 func, uint32 *eax, uint32 *ebx, uint32 *ecx, uint32 *edx) */
FUNCTION(i386_cpuid):
    pushl %ebx          /* store ebx (ARG0 entry occupied by ebx) */
    pushl %edi          /* store edi (ARG1 entry occupied by edi) */
    movl ARG2, %eax     /* load CPUID function number into eax */
    cpuid               /* execute CPUID instruction */
    movl ARG3, %edi     /* store CPUID output */
    movl %eax, (%edi)   /* eax */
    movl ARG4, %edi
    movl %ebx, (%edi)   /* ebx */
    movl ARG5, %edi
    movl %ecx, (%edi)   /* ecx */
    movl ARG6, %edi
    movl %edx, (%edi)   /* edx */
    popl %edi           /* restore edi */
    popl %ebx           /* restore ebx */
    ret

/* uint32 i386_cpuid_eax(uint32 func) */
FUNCTION(i386_cpuid_eax):
    pushl %ebx          /* store ebx (ARG0 entry occupied by ebx) */
    movl ARG1, %eax     /* load CPUID function number into eax */
    cpuid               /* execute CPUID instruction */
    popl %ebx           /* restore ebx */
    ret

/* uint32 i386_cpuid_ebx(uint32 func) */
FUNCTION(i386_cpuid_ebx):
    pushl %ebx          /* store ebx (ARG0 entry occupied by ebx) */
    movl ARG1, %eax     /* load CPUID function number into eax */
    cpuid               /* execute CPUID instruction */
    movl %ebx, %eax     /* copy ebx to eax */
    popl %ebx           /* restore ebx */
    ret

/* uint32 i386_cpuid_ecx(uint32 func) */
FUNCTION(i386_cpuid_ecx):
    pushl %ebx          /* store ebx (ARG0 entry occupied by ebx) */
    movl ARG1, %eax     /* load CPUID function number into eax */
    cpuid               /* execute CPUID instruction */
    movl %ecx, %eax     /* copy ecx to eax */
    popl %ebx           /* restore ebx */
    ret

/* uint32 i386_cpuid_edx(uint32 func) */
FUNCTION(i386_cpuid_edx):
    pushl %ebx          /* store ebx (ARG0 entry occupied by ebx) */
    movl ARG1, %eax     /* load CPUID function number into eax */
    cpuid               /* execute CPUID instruction */
    movl %edx, %eax     /* copy edx to eax */
    popl %ebx           /* restore ebx */
    ret

/* uint64 i386_rdtsc() */
FUNCTION(i386_rdtsc):
    rdtsc   /* read processor time-stamp counter into edx:eax */
    ret

#if CPU_i386
/*
 * CPU i386 compatible versions.
 * Routines invalidates whole TLB by reloading CR3. It is not
 * possible to invalidate single TLB entry because INVLPG
 * instruction supported only by i486+ CPUs.
 */
FUNCTION(cpu_i386_arch_invalidate_TLB_entry):
FUNCTION(cpu_i386_arch_invalidate_TLB_range):
FUNCTION(cpu_i386_arch_invalidate_TLB_list):
    movl  %cr3,  %eax   /* copy %cr3 to %eax */
    movl  %eax,  %cr3   /* copy %eax back to %cr3 */
    ret

/* i486+ CPUs version */
FUNCTION(cpu_i486_arch_invalidate_TLB_entry):
    movl    ARG0,  %eax
    invlpg  (%eax)  /* invalidate single TLB entry */
    ret

/* Links to correct routines will be set during CPU identification */
GLOBAL(__arch_invalidate_TLB_entry_link): .long 0
FUNCTION(arch_invalidate_TLB_entry):
    jmp *__arch_invalidate_TLB_entry_link
    
GLOBAL(__arch_invalidate_TLB_range_link): .long 0
FUNCTION(arch_invalidate_TLB_range):
    jmp *__arch_invalidate_TLB_range_link

GLOBAL(__arch_invalidate_TLB_list_link): .long 0
FUNCTION(arch_invalidate_TLB_list):
    jmp *__arch_invalidate_TLB_list_link
#endif
