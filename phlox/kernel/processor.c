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

typedef void (*mod_init)(processor_t *bsp);


void processor_set_init(kernel_args_t *kargs, uint32 curr_cpu) {
    if(curr_cpu == 0) {
       memset(&ProcessorSet, 0, sizeof(processor_set_t));
       arch_processor_set_init(&ProcessorSet.arch, kargs, curr_cpu);

       processor_init(&ProcessorSet.processors[curr_cpu], kargs, curr_cpu);
    } else {
       panic("processor_set_init: SMP is not supported!\n");
    }
    panic("Processor set init finished!\n");
}

void processor_init(processor_t *p, kernel_args_t *kargs, uint32 curr_cpu) {
    arch_processor_init(&p->arch, kargs, curr_cpu);
}
