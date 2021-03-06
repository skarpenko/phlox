/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <phlox/kernel.h>
#include <phlox/kargs.h>
#include <phlox/errors.h>
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


void processor_set_init(kernel_args_t *kargs, uint curr_cpu)
{
    uint i;

    if(curr_cpu == 0) { /* we are bootstrap processor */
       /* prepare ProcessorSet */
       memset(&ProcessorSet, 0, sizeof(processor_set_t));
       /* set processors number */
       ProcessorSet.processors_num = kargs->num_cpus;
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

status_t processor_set_init_after_vm(kernel_args_t *kargs, uint curr_cpu)
{
    status_t err;

    if(curr_cpu == 0) { /* we are bootstrap cpu */
        /* call architecture specific version */
        err = arch_processor_set_init_after_vm(&ProcessorSet.arch, kargs, curr_cpu);
        if(err != NO_ERROR)
            panic("arch_processor_set_init_after_vm: FAILED!\n");

        /* per processor init for bootstrap cpu */
        err = processor_init_after_vm(&ProcessorSet.processors[curr_cpu], kargs, curr_cpu);
        if(err != NO_ERROR)
            panic("arch_processor_init_after_vm: FAILED! (CPU%d)\n", curr_cpu);
    } else {
        /* NOTE: Slave CPUs waiting until bootstrap CPU completes */

        /* per processor init */
        err = processor_init_after_vm(&ProcessorSet.processors[curr_cpu], kargs, curr_cpu);
        if(err != NO_ERROR)
            panic("arch_processor_init_after_vm: FAILED! (CPU%d)\n", curr_cpu);
    }

    return NO_ERROR;
}

void processor_init(processor_t *p, kernel_args_t *kargs, uint curr_cpu)
{
    p->cpu_num = curr_cpu; /* set CPU number */

    /* execute architecture specific inits */
    arch_processor_init(&p->arch, kargs, curr_cpu);
}

status_t processor_init_after_vm(processor_t *p, kernel_args_t *kargs, uint curr_cpu)
{
    status_t err;

    /* architecture specific init */
    err = arch_processor_init_after_vm(&p->arch, kargs, curr_cpu);

    return err;
}

void processor_idle_cycle(void)
{
    while(true)
        safe_halt(); /* stop cpu until interrupt occurs */
}

uint get_current_processor(void)
{
    /* As SMP is not currently supported we always return
     * Bootstrap processor number. With SMP support this
     * routine might be moved to special SMP module.
     */
    return BOOTSTRAP_CPU;

/* And fire compilation error if SMP support requested */
#if SYSCFG_SMP_SUPPORT
#  error get_current_processor(): smp not supported!
#endif
}

processor_t *get_current_processor_struct(void)
{
    return &ProcessorSet.processors[get_current_processor()];
}
