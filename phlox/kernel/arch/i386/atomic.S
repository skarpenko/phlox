/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/

/* Print warning if compiling for i386 CPU */
#if CPU_i386
#warning Compiling for i386 CPU
#endif

#define FUNCTION(x) .global x; .type x,@function; x
#define GLOBAL(x) .global x; x

/* Use lock prefix if compiling for SMP */
#if SYSCFG_SMP_SUPPORT
  #define LOCK lock
#else
  #define LOCK
#endif

/* Arguments */
#define ARG0   4(%esp)
#define ARG1   8(%esp)
#define ARG2  12(%esp)

.text

/* void atomic_set(atomic_t *a, int set_to) */
FUNCTION(atomic_set):
    movl  ARG0, %edx
    movl  ARG1, %eax
    LOCK                /* Lock bus */
    movl  %eax, (%edx)
    ret

/* int atomic_set_ret(atomic_t *a, int set_to) */
FUNCTION(atomic_set_ret):
    movl  ARG0, %edx
    movl  ARG1, %eax
    xchg  %eax, (%edx)  /* Exchange %eax and (%edx). Bus locked by xchg instruction. */
    ret

/* int atomic_test_and_set(atomic_t *a, int set_to, int test_val) */
#if CPU_i386
/* NOTE: Link to correct routine will be set during cpu detection stage. */
GLOBAL(__atomic_test_and_set_link): .long 0
FUNCTION(atomic_test_and_set):
    jmp *__atomic_test_and_set_link

FUNCTION(cpu_i386_atomic_test_and_set):
    movl   ARG0, %edx
    movl   ARG1, %ecx
    movl   ARG2, %eax
    /* cpu i386 implementation of cmpxchg instruction */
    pushfl               /* store flags */
    cli                  /* disable interrupts */
    cmpl   %eax, (%edx)  /* compare %eax and (%edx) */
    jnz    1f
    xchg   %ecx, (%edx)  /* exchange %ecx and (%edx) if %eax = (%edx) */
1:  popfl                /* restore flags */
    movl   $0,   %eax
    sete   %al           /* Set %eax = 1 if %ecx written to (%edx) */
    ret

FUNCTION(cpu_i486_atomic_test_and_set):
#else
FUNCTION(atomic_test_and_set):
#endif
    movl      ARG0, %edx
    movl      ARG1, %ecx
    movl      ARG2, %eax
    LOCK                    /* Lock bus */
    cmpxchgl  %ecx, (%edx)  /* Write %ecx to (%edx) if %eax = (%edx) */
    movl      $0,   %eax
    sete      %al           /* Set %eax = 1 if %ecx written to (%edx) */
    ret

/* int atomic_get(atomic_t *a) */
FUNCTION(atomic_get):
    movl  ARG0,   %edx
    LOCK                /* Lock bus */
    movl  (%edx), %eax
    ret

/* void atomic_add(atomic_t *a, int v) */
FUNCTION(atomic_add):
    movl  ARG0, %edx
    movl  ARG1, %eax
    LOCK                /* Lock bus */
    addl  %eax, (%edx)  /* Add %eax to (%edx) */
    ret

/* void atomic_sub(atomic_t *a, int v) */
FUNCTION(atomic_sub):
    movl  ARG0, %edx
    movl  ARG1, %eax
    LOCK                /* Lock bus */
    subl  %eax, (%edx)  /* Subtract %eax from (%edx) */
    ret

/* void atomic_inc(atomic_t *a) */
FUNCTION(atomic_inc):
    movl  ARG0, %edx
    LOCK              /* Lock bus */
    incl  (%edx)      /* Increment (%edx) by 1 */
    ret

/* void atomic_dec(atomic_t *a) */
FUNCTION(atomic_dec):
    movl  ARG0, %edx
    LOCK              /* Lock bus */
    decl  (%edx)      /* Decrement (%edx) by 1 */
    ret

/* void atomic_neg(atomic_t *a) */
FUNCTION(atomic_neg):
    movl  ARG0, %edx
    LOCK              /* Lock bus */
    negl  (%edx)      /* Change sign of (%edx) */
    ret

/* int atomic_add_ret(atomic_t *a, int v) */
#if CPU_i386
/* NOTE: Link to correct routine will be set during cpu detection stage. */
GLOBAL(__atomic_add_ret_link): .long 0
FUNCTION(atomic_add_ret):
    jmp *__atomic_add_ret_link

FUNCTION(cpu_i386_atomic_add_ret):
    movl   ARG0,   %edx
    movl   ARG1,   %eax
    /* cpu i386 implementation of xadd instruction */
    pushfl                 /* store flags */
    cli                    /* disable interrupts */
    movl   (%edx), %ecx
    addl   %ecx,   %eax    /* add */
    xchg   %eax,   (%edx)  /* exchange */
    popfl                  /* restore flags */
    ret

FUNCTION(cpu_i486_atomic_add_ret):
#else
FUNCTION(atomic_add_ret):
#endif
    movl   ARG0, %edx
    movl   ARG1, %eax
    LOCK                 /* Lock bus */
    xaddl  %eax, (%edx)  /* Add %eax to (%edx) and store previous value in %eax */
    ret

/* int atomic_sub_ret(atomic_t *a, int v) */
#if CPU_i386
/* NOTE: Link to correct routine will be set during cpu detection stage. */
GLOBAL(__atomic_sub_ret_link): .long 0
FUNCTION(atomic_sub_ret):
    jmp *__atomic_sub_ret_link

FUNCTION(cpu_i386_atomic_sub_ret):
    movl   ARG0,   %edx
    movl   ARG1,   %ecx
    pushfl                 /* store flags */
    cli                    /* disable interrupts */
    movl   (%edx), %eax
    subl   %ecx,   %eax    /* subtract */
    xchg   %eax,   (%edx)  /* exchange */
    popfl                  /* restore flags */
    ret

FUNCTION(cpu_i486_atomic_sub_ret):
#else
FUNCTION(atomic_sub_ret):
#endif
    movl   ARG0, %edx
    movl   ARG1, %eax
    negl   %eax          /* Change sign of %eax */
    LOCK                 /* Lock bus */
    xaddl  %eax, (%edx)  /* Add %eax to (%edx) and store previous value in %eax */
    ret

/* int atomic_inc_ret(atomic_t *a) */
#if CPU_i386
/* NOTE: Link to correct routine will be set during cpu detection stage. */
GLOBAL(__atomic_inc_ret_link): .long 0
FUNCTION(atomic_inc_ret):
    jmp *__atomic_inc_ret_link

FUNCTION(cpu_i386_atomic_inc_ret):
    movl   ARG0,   %edx
    movl   $1,     %eax    /* Write 1 to %eax */
    /* cpu i386 implementation of xadd instruction */
    pushfl                 /* store flags */
    cli                    /* disable interrupts */
    movl   (%edx), %ecx
    addl   %ecx,   %eax    /* add */
    xchg   %eax,   (%edx)  /* exchange */
    popfl                  /* restore flags */
    ret

FUNCTION(cpu_i486_atomic_inc_ret):
#else
FUNCTION(atomic_inc_ret):
#endif
    movl   ARG0, %edx
    movl   $1,   %eax    /* Write 1 to %eax */
    LOCK                 /* Lock bus */
    xaddl  %eax, (%edx)  /* Add %eax to (%edx) and store previous value in %eax */
    ret

/* int atomic_dec_ret(atomic_t *a) */
#if CPU_i386
/* NOTE: Link to correct routine will be set during cpu detection stage. */
GLOBAL(__atomic_dec_ret_link): .long 0
FUNCTION(atomic_dec_ret):
    jmp *__atomic_dec_ret_link

FUNCTION(cpu_i386_atomic_dec_ret):
    movl   ARG0,   %edx
    movl   $-1,    %eax    /* Write -1 to %eax */
    /* cpu i386 implementation of xadd instruction */
    pushfl                 /* store flags */
    cli                    /* disable interrupts */
    movl   (%edx), %ecx
    addl   %ecx,   %eax    /* add */
    xchg   %eax,   (%edx)  /* exchange */
    popfl                  /* restore flags */
    ret

FUNCTION(cpu_i486_atomic_dec_ret):
#else
FUNCTION(atomic_dec_ret):
#endif
    movl   ARG0, %edx
    movl   $-1,  %eax    /* Write -1 to %eax */
    LOCK                 /* Lock bus */
    xaddl  %eax, (%edx)  /* Add %eax to (%edx) and store previous value in %eax */
    ret

/* int atomic_neg_ret(atomic_t *a) */
#if CPU_i386
/* NOTE: Link to correct routine will be set during cpu detection stage. */
GLOBAL(__atomic_neg_ret_link): .long 0
FUNCTION(atomic_neg_ret):
    jmp *__atomic_neg_ret_link

FUNCTION(cpu_i386_atomic_neg_ret):
    movl   ARG0,   %edx
_cpu_i386_atomic_neg_ret1:
    movl   (%edx), %eax    /* Write (%edx) to %eax */
    movl   %eax,   %ecx    /* Make a copy of %eax in %ecx */
    negl   %ecx            /* Change sign of %ecx */
    /* cpu i386 implementation of cmpxchg instruction */
    pushfl                 /* store flags */
    cli                    /* disable interrupts */
    cmpl   %eax,   (%edx)  /* compare %eax and (%edx) */
    jnz    1f
    xchg   %ecx,   (%edx)  /* exchange %ecx and (%edx) if %eax = (%edx) */
1:  popfl                  /* restore flags */
    jnz    _cpu_i386_atomic_neg_ret1  /* else try again */
    ret

FUNCTION(cpu_i486_atomic_neg_ret):
#else
FUNCTION(atomic_neg_ret):
#endif
    movl      ARG0,   %edx
_atomic_neg_ret1:
    movl      (%edx), %eax      /* Write (%edx) to %eax */
    movl      %eax,   %ecx      /* Make a copy of %eax in %ecx */
    negl      %ecx              /* Change sign of %ecx */
    LOCK                        /* Lock bus */
    cmpxchgl  %ecx, (%edx)      /* Exchange if value in (%edx) not changed */
    jnz       _atomic_neg_ret1  /* else try again */
    ret
    
/* void atomic_not(atomic_t *a) */
FUNCTION(atomic_not):
    movl  ARG0, %edx
    LOCK              /* Lock bus */
    notl  (%edx)      /* Logical NOT */
    ret

/* void atomic_and(atomic_t *a, int v) */
FUNCTION(atomic_and):
    movl  ARG0, %edx
    movl  ARG1, %eax
    LOCK                /* Lock bus */
    andl  %eax, (%edx)  /* Logical AND */
    ret

/* void atomic_or(atomic_t *a, int v) */
FUNCTION(atomic_or):
    movl  ARG0, %edx
    movl  ARG1, %eax
    LOCK                /* Lock bus */
    orl  %eax,  (%edx)  /* Logical OR */
    ret

/* void atomic_xor(atomic_t *a, int v) */
FUNCTION(atomic_xor):
    movl  ARG0, %edx
    movl  ARG1, %eax
    LOCK                /* Lock bus */
    xorl  %eax, (%edx)  /* Logical XOR */
    ret

/* int atomic_not_ret(atomic_t *a) */
#if CPU_i386
/* NOTE: Link to correct routine will be set during cpu detection stage. */
GLOBAL(__atomic_not_ret_link): .long 0
FUNCTION(atomic_not_ret):
    jmp *__atomic_not_ret_link

FUNCTION(cpu_i386_atomic_not_ret):
    movl   ARG0,   %edx
_cpu_i386_atomic_not_ret1:
    movl   (%edx), %eax      /* Write (%edx) to %eax */
    movl   %eax,   %ecx      /* Make a copy of %eax in %ecx */
    notl   %ecx              /* Logical NOT */
    /* cpu i386 implementation of cmpxchg instruction */
    pushfl                   /* store flags */
    cli                      /* disable interrupts */
    cmpl   %eax,   (%edx)    /* compare %eax and (%edx) */
    jnz    1f
    xchg   %ecx,   (%edx)    /* exchange %ecx and (%edx) if %eax = (%edx) */
1:  popfl                    /* restore flags */
    jnz    _cpu_i386_atomic_not_ret1  /* else try again */
    ret

FUNCTION(cpu_i486_atomic_not_ret):
#else
FUNCTION(atomic_not_ret):
#endif
    movl      ARG0,   %edx
_atomic_not_ret1:
    movl      (%edx), %eax      /* Write (%edx) to %eax */
    movl      %eax,   %ecx      /* Make a copy of %eax in %ecx */
    notl      %ecx              /* Logical NOT */
    LOCK                        /* Lock bus */
    cmpxchgl  %ecx,   (%edx)    /* Exchange if value in (%edx) not changed */
    jnz       _atomic_not_ret1  /* else try again */
    ret

/* int atomic_and_ret(atomic_t *a, int v) */
#if CPU_i386
/* NOTE: Link to correct routine will be set during cpu detection stage. */
GLOBAL(__atomic_and_ret_link): .long 0
FUNCTION(atomic_and_ret):
    jmp *__atomic_and_ret_link

FUNCTION(cpu_i386_atomic_and_ret):
    movl    ARG0,   %edx
_cpu_i386_atomic_and_ret1:
    movl    ARG1,   %ecx
    movl    (%edx), %eax      /* Copy (%edx) to %eax */
    andl    %eax,   %ecx      /* Logical AND */
    /* cpu i386 implementation of cmpxchg instruction */
    pushfl                    /* store flags */
    cli                       /* disable interrupts */
    cmpl    %eax,   (%edx)    /* compare %eax and (%edx) */
    jnz    1f
    xchg   %ecx,    (%edx)    /* exchange %ecx and (%edx) if %eax = (%edx) */
1:  popfl                     /* restore flags */
    jnz    _cpu_i386_atomic_and_ret1  /* else try again */
    ret

FUNCTION(cpu_i486_atomic_and_ret):
#else
FUNCTION(atomic_and_ret):
#endif
    movl      ARG0,   %edx
_atomic_and_ret1:
    movl      ARG1,   %ecx
    movl      (%edx), %eax      /* Copy (%edx) to %eax */
    andl      %eax,   %ecx      /* Logical AND */
    LOCK                        /* Lock bus */
    cmpxchgl  %ecx,   (%edx)    /* Exchange if value in (%edx) not changed */
    jnz       _atomic_and_ret1  /* else try again */
    ret

/* int atomic_or_ret(atomic_t *a, int v) */
#if CPU_i386
/* NOTE: Link to correct routine will be set during cpu detection stage. */
GLOBAL(__atomic_or_ret_link): .long 0
FUNCTION(atomic_or_ret):
    jmp *__atomic_or_ret_link

FUNCTION(cpu_i386_atomic_or_ret):
    movl   ARG0,   %edx
_cpu_i386_atomic_or_ret1:
    movl   ARG1,   %ecx
    movl   (%edx), %eax      /* Copy (%edx) to %eax */
    orl    %eax,   %ecx      /* Logical OR */
    /* cpu i386 implementation of cmpxchg instruction */
    pushfl                   /* store flags */
    cli                      /* disable interrupts */
    cmpl   %eax,   (%edx)    /* compare %eax and (%edx) */
    jnz 1f
    xchg   %ecx,   (%edx)    /* exchange %ecx and (%edx) if %eax = (%edx) */
1:  popfl                    /* restore flags */
    jnz    _cpu_i386_atomic_or_ret1   /* else try again */
    ret

FUNCTION(cpu_i486_atomic_or_ret):
#else
FUNCTION(atomic_or_ret):
#endif
    movl      ARG0,   %edx
_atomic_or_ret1:
    movl      ARG1,   %ecx
    movl      (%edx), %eax      /* Copy (%edx) to %eax */
    orl       %eax,   %ecx      /* Logical OR */
    LOCK                        /* Lock bus */
    cmpxchgl  %ecx,   (%edx)    /* Exchange if value in (%edx) not changed */
    jnz       _atomic_or_ret1   /* else try again */
    ret

/* int atomic_xor_ret(atomic_t *a, int v) */
#if CPU_i386
/* NOTE: Link to correct routine will be set during cpu detection stage. */
GLOBAL(__atomic_xor_ret_link): .long 0
FUNCTION(atomic_xor_ret):
    jmp *__atomic_xor_ret_link

FUNCTION(cpu_i386_atomic_xor_ret):
    movl   ARG0,   %edx
_cpu_i386_atomic_xor_ret1:
    movl   ARG1,   %ecx
    movl   (%edx), %eax      /* Copy (%edx) to %eax */
    xorl   %eax,   %ecx      /* Logical XOR */
    /* cpu i386 implementation of cmpxchg instruction */
    pushfl                   /* store flags */
    cli                      /* disable interrupts */
    cmpl   %eax,   (%edx)    /* compare %eax and (%edx) */
    jnz    1f
    xchg   %ecx,   (%edx)    /* exchange %ecx and (%edx) if %eax = (%edx) */
1:  popfl                    /* restore flags */
    jnz    _cpu_i386_atomic_xor_ret1  /* else try again */
    ret

FUNCTION(cpu_i486_atomic_xor_ret):
#else
FUNCTION(atomic_xor_ret):
#endif
    movl      ARG0,   %edx
_atomic_xor_ret1:
    movl      ARG1,   %ecx
    movl      (%edx), %eax      /* Copy (%edx) to %eax */
    xorl      %eax,   %ecx      /* Logical XOR */
    LOCK                        /* Lock bus */
    cmpxchgl  %ecx,   (%edx)    /* Exchange if value in (%edx) not changed */
    jnz       _atomic_xor_ret1  /* else try again */
    ret

/* int atomic_add_and_testz(atomic_t *a, int v) */
FUNCTION(atomic_add_and_testz):
    movl  ARG0, %edx
    movl  ARG1, %eax
    LOCK                /* Lock bus */
    addl  %eax, (%edx)  /* Add %eax to (%edx) */
    movl  $0,   %eax
    sete  %al           /* Set %eax = 1 if result is zero */
    ret

/* int atomic_sub_and_testz(atomic_t *a, int v) */
FUNCTION(atomic_sub_and_testz):
    movl  ARG0, %edx
    movl  ARG1, %eax
    LOCK                /* Lock bus */
    subl  %eax, (%edx)  /* Subtract %eax from (%edx) */
    movl  $0,   %eax
    sete  %al           /* Set %eax = 1 if result is zero */
    ret

/* int atomic_inc_and_testz(atomic_t *a) */
FUNCTION(atomic_inc_and_testz):
    movl  ARG0, %edx
    LOCK              /* Lock bus */
    incl  (%edx)      /* Icrement (%edx) by 1 */
    movl  $0,   %eax
    sete  %al         /* Set %eax = 1 if result is zero */
    ret

/* int atomic_dec_and_testz(atomic_t *a) */
FUNCTION(atomic_dec_and_testz):
    movl  ARG0, %edx
    LOCK              /* Lock bus */
    decl  (%edx)      /* Decrement (%edx) by 1 */
    movl  $0,   %eax
    sete  %al         /* Set %eax = 1 if result is zero */
    ret

/* int atomic_add_and_tests(atomic_t *a, int v) */
FUNCTION(atomic_add_and_tests):
    movl  ARG0, %edx
    movl  ARG1, %eax
    LOCK                /* Lock bus */
    addl  %eax, (%edx)  /* Add %eax to (%edx) */
    movl  $0,   %eax
    sets  %al           /* Set %eax = 1 if result is negative */
    ret

/* int atomic_sub_and_tests(atomic_t *a, int v) */
FUNCTION(atomic_sub_and_tests):
    movl  ARG0, %edx
    movl  ARG1, %eax
    LOCK                /* Lock bus */
    subl  %eax, (%edx)  /* Subtract %eax from (%edx) */
    movl  $0,   %eax
    sets  %al           /* Set %eax = 1 if result is negative */
    ret

/* int atomic_inc_and_tests(atomic_t *a) */
FUNCTION(atomic_inc_and_tests):
    movl  ARG0, %edx
    LOCK            /* Lock bus */
    incl  (%edx)    /* Increment (%edx) by 1 */
    movl  $0, %eax
    sets  %al       /* Set %eax = 1 if result is negative */
    ret

/* int atomic_dec_and_tests(atomic_t *a) */
FUNCTION(atomic_dec_and_tests):
    movl  ARG0, %edx
    LOCK            /* Lock bus */
    decl  (%edx)    /* Decrement (%edx) by 1 */
    movl  $0, %eax
    sets  %al       /* Set %eax = 1 if result is negative */
    ret
