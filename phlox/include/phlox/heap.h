/*
* Copyright 2007-2011, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_HEAP_H
#define _PHLOX_HEAP_H

#include <phlox/types.h>
#include <phlox/kernel.h>
#include <phlox/kargs.h>

/* called from vm_init. The heap should already be mapped in at this point,
 * we just do a little housekeeping to set up the data structure.
*/
status_t heap_init(addr_t new_heap_base, size_t new_heap_size);

/* called after threading initiated */
status_t heap_init_postthread(kernel_args_t *ka);

/* called after semaphores initiated */
status_t heap_init_postsem(kernel_args_t *ka);

/* kernel malloc */
void *kmalloc(size_t size);

/* kernel free */
void kfree(void *address);

/* kfree and set to NULL */
void kfree_and_null(void **address);

/*
 * Allocates npages of hardware pages and guarantees allocated
 * memory page aligned.
 */
void *kmalloc_pages(size_t npages);

#endif
