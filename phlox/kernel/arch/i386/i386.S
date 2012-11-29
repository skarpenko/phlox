/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/

#define FUNCTION(x) .global x; .type x,@function; x

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
