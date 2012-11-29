/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <phlox/kernel.h>
#include <phlox/kargs.h>
#include <phlox/processor.h>


/* global processor set structure */
processor_set_t  ProcessorSet;


/* pointer to module init routine */
typedef void (*mod_init_t)(arch_processor_t *bsp);

/*** external module initialization routines ***/
void arch_atomic_mod_init(arch_processor_t *bsp);

/* modules init list */
static mod_init_t mod_inits[] = {
    &arch_processor_mod_init,   /* processor module init */
    &arch_atomic_mod_init,      /* atomic module init */

    NULL /* last entry */
};


void processor_set_init(kernel_args_t *kargs, uint32 curr_cpu) {
    uint32 i;
    
    if(curr_cpu == 0) { /* we are bootstrap processor */
       /* prepare ProcessorSet */
       memset(&ProcessorSet, 0, sizeof(processor_set_t));
       /* execute architecture specific inits */
       arch_processor_set_init(&ProcessorSet.arch, kargs, curr_cpu);
       /* execute processor init */
       processor_init(&ProcessorSet.processors[curr_cpu], kargs, curr_cpu);

       /* execute modules init sequence */
       i = 0;
       while(mod_inits[i]) {
          /* pass bootstrap arch_processor structure as a parameter */
          mod_inits[i](&ProcessorSet.processors[curr_cpu].arch);
          i++;
       }
    } else {
       /* currently SMP is not supported */
       panic("processor_set_init: SMP is not supported!\n");
    }
}

void processor_init(processor_t *p, kernel_args_t *kargs, uint32 curr_cpu) {
    /* execute architecture specific inits */
    arch_processor_init(&p->arch, kargs, curr_cpu);
}
