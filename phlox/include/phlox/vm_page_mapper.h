/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_VM_PAGE_MAPPER_H_
#define _PHLOX_VM_PAGE_MAPPER_H_

#include <phlox/ktypes.h>
#include <phlox/kernel.h>
#include <phlox/kargs.h>

/* map chunk function type */
typedef status_t (*map_chunk_func_t)(addr_t,addr_t);


/*
 * Page mapper initialization. Called during system init.
 *
 * pool_base - pointer to variable where mappings pool base address
 *             be stored;
 * pool_size - mappings pool size (must be pool_chunksize aligned);
 * pool_chunksize - mappings pool chunk size;
 * map_chunk_func - chunk mapper function;
 *
 * NOTE: pool_base address will be aligned to pool_align boundary.
 */
status_t vm_page_mapper_init(kernel_args_t *kargs, addr_t *pool_base, size_t pool_size,
                             size_t pool_chunksize, size_t pool_align,
                             map_chunk_func_t map_chunk_func);

/*
 * Get physical page.
 * If no free slots in mappings pool and can_wait is true then
 * caller thread be suspended until free slots appears.
 * If can_wait is false, error returned.
 */
status_t vm_pmap_get_ppage(addr_t pa, addr_t *va, bool can_wait);

/*
 * Put physical page, previously taken by vm_pmap_get_ppage
 */
status_t vm_pmap_put_ppage(addr_t va);

#endif
