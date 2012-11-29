/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <strings.h>
#include <phlox/kargs.h>
#include <phlox/processor.h>

/* global processor set structure */
processor_set_t  ProcessorSet;

typedef void (*mod_init)(processor_t *bsp);


void processor_set_init(kernel_args_t *kargs, uint32 curr_cpu) {
}

void processor_init(processor_t *p, kernel_args_t *kargs, uint32 curr_cpu) {
}
