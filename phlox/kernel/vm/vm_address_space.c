/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <phlox/errors.h>
#include <phlox/heap.h>
#include <phlox/list.h>
#include <phlox/atomic.h>
#include <phlox/spinlock.h>
#include <phlox/arch/vm_translation_map.h>
#include <phlox/vm_page.h>
#include <phlox/vm_types.h>
#include <phlox/vm.h>


/* Redefinition for convenience */
typedef xlist_t aspace_list_t;

/* Next available address space id */
static vuint next_aspace_id;

/* List of address spaces */
static aspace_list_t aspaces_list;

/* Spinlock for operations on address spaces list */
static spinlock_t aspace_lock;

/* Kernel address space */
static vm_address_space_t *kernel_aspace = NULL;


/* returns available address space id */
static aspace_id get_next_aspace_id(void)
{
    aspace_id retval;

    /* atomically increment and get previous value */
    retval = (aspace_id)atomic_inc_ret(&next_aspace_id);
    if(retval == VM_INVALID_ASPACEID)
        panic("No available address space IDs!");

    return retval;
}


/* Module initialization routine */
status_t vm_address_spaces_init(kernel_args_t *kargs); /* keep silence */
status_t vm_address_spaces_init(kernel_args_t *kargs)
{
    /* set initial value */
    next_aspace_id = 1;

    /* init address spaces list */
    xlist_init(&aspaces_list);

    return NO_ERROR;
}
