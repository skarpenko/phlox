/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
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

/* ulong __syscall0(ulong sys_no) */
FUNCTION(__syscall0):
    pushl %ebx
    pushl %esi
    jmp syscall_start

/* ulong __syscall1(ulong sys_no, ulong arg0) */
FUNCTION(__syscall1):
    pushl %ebx
    pushl %esi
    movl ARG3, %ebx
    jmp syscall_start

/* ulong __syscall2(ulong sys_no, ulong arg0, ulong arg1) */
FUNCTION(__syscall2):
    pushl %ebx
    pushl %esi
    movl ARG3, %ebx
    movl ARG4, %ecx
    jmp syscall_start

/* ulong __syscall3(ulong sys_no, ulong arg0, ulong arg1, ulong arg2) */
FUNCTION(__syscall3):
    pushl %ebx
    pushl %esi
    movl ARG3, %ebx
    movl ARG4, %ecx
    movl ARG5, %edx
    jmp syscall_start

/* ulong __syscall4(ulong sys_no, ulong arg0, ulong arg1, ulong arg2, ulong arg3) */
FUNCTION(__syscall4):
    pushl %ebx
    pushl %esi
    movl ARG3, %ebx
    movl ARG4, %ecx
    movl ARG5, %edx
    movl ARG6, %esi
    jmp syscall_start

syscall_start:
    movl ARG2, %eax

    int $0xC0 /* do system call */

    popl %esi
    popl %ebx
    ret
