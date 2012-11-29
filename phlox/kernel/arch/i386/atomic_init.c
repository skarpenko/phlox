/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/types.h>
#include <phlox/processor.h>

#if CPU_i386
/* CPU dependend routines */
extern uint32 __atomic_test_and_set_link;
int cpu_i386_atomic_test_and_set(atomic_t *a, int set_to, int test_val);
int cpu_i486_atomic_test_and_set(atomic_t *a, int set_to, int test_val);

extern uint32 __atomic_add_ret_link;
int cpu_i386_atomic_add_ret(atomic_t *a, int v);
int cpu_i486_atomic_add_ret(atomic_t *a, int v);

extern uint32 __atomic_sub_ret_link;
int cpu_i386_atomic_sub_ret(atomic_t *a, int v);
int cpu_i486_atomic_sub_ret(atomic_t *a, int v);

extern uint32 __atomic_inc_ret_link;
int cpu_i386_atomic_inc_ret(atomic_t *a);
int cpu_i486_atomic_inc_ret(atomic_t *a);

extern uint32 __atomic_dec_ret_link;
int cpu_i386_atomic_dec_ret(atomic_t *a);
int cpu_i486_atomic_dec_ret(atomic_t *a);

extern uint32 __atomic_neg_ret_link;
int cpu_i386_atomic_neg_ret(atomic_t *a);
int cpu_i486_atomic_neg_ret(atomic_t *a);
    
extern uint32 __atomic_not_ret_link;
int cpu_i386_atomic_not_ret(atomic_t *a);
int cpu_i486_atomic_not_ret(atomic_t *a);

extern uint32 __atomic_and_ret_link;
int cpu_i386_atomic_and_ret(atomic_t *a, int v);
int cpu_i486_atomic_and_ret(atomic_t *a, int v);

extern uint32 __atomic_or_ret_link;
int cpu_i386_atomic_or_ret(atomic_t *a, int v);
int cpu_i486_atomic_or_ret(atomic_t *a, int v);

extern uint32 __atomic_xor_ret_link;
int cpu_i386_atomic_xor_ret(atomic_t *a, int v);
int cpu_i486_atomic_xor_ret(atomic_t *a, int v);
#endif

/* atomic module init routine */
void arch_atomic_mod_init(arch_processor_t *bsp); /* keep compiler happy */
void arch_atomic_mod_init(arch_processor_t *bsp)
{
#if CPU_i386
    if(bsp->family<4) {
       /* init links for i386 CPU */
       __atomic_test_and_set_link = (uint32)&cpu_i386_atomic_test_and_set;
       __atomic_add_ret_link      = (uint32)&cpu_i386_atomic_add_ret;
       __atomic_sub_ret_link      = (uint32)&cpu_i386_atomic_sub_ret;
       __atomic_inc_ret_link      = (uint32)&cpu_i386_atomic_inc_ret;
       __atomic_dec_ret_link      = (uint32)&cpu_i386_atomic_dec_ret;
       __atomic_neg_ret_link      = (uint32)&cpu_i386_atomic_neg_ret;
       __atomic_not_ret_link      = (uint32)&cpu_i386_atomic_not_ret;
       __atomic_and_ret_link      = (uint32)&cpu_i386_atomic_and_ret;
       __atomic_or_ret_link       = (uint32)&cpu_i386_atomic_or_ret;
       __atomic_xor_ret_link      = (uint32)&cpu_i386_atomic_xor_ret;
    } else {
       /* init links for i486+ CPU */
       __atomic_test_and_set_link = (uint32)&cpu_i486_atomic_test_and_set;
       __atomic_add_ret_link      = (uint32)&cpu_i486_atomic_add_ret;
       __atomic_sub_ret_link      = (uint32)&cpu_i486_atomic_sub_ret;
       __atomic_inc_ret_link      = (uint32)&cpu_i486_atomic_inc_ret;
       __atomic_dec_ret_link      = (uint32)&cpu_i486_atomic_dec_ret;
       __atomic_neg_ret_link      = (uint32)&cpu_i486_atomic_neg_ret;
       __atomic_not_ret_link      = (uint32)&cpu_i486_atomic_not_ret;
       __atomic_and_ret_link      = (uint32)&cpu_i486_atomic_and_ret;
       __atomic_or_ret_link       = (uint32)&cpu_i486_atomic_or_ret;
       __atomic_xor_ret_link      = (uint32)&cpu_i486_atomic_xor_ret;
    }
#endif
}
