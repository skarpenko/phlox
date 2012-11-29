/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_HEAP_H
#define _PHLOX_HEAP_H

#include <phlox/kernel.h>
#include <phlox/kargs.h>

/* called from vm_init. The heap should already be mapped in at this point,
 * we just do a little housekeeping to set up the data structure.
*/
uint32 heap_init(addr_t new_heap_base, size_t new_heap_size);

/* called after semaphores initiated */
uint32 heap_init_postsem(kernel_args_t *ka);

/* kernel malloc */
void *kmalloc(size_t size);

/* kernel free */
void kfree(void *address);

/* kfree and set to NULL */
void kfree_and_null(void **address);

#endif
