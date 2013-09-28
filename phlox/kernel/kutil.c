/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <phlox/errors.h>
#include <phlox/types.h>
#include <phlox/kernel.h>
#include <phlox/vm.h>
#include <phlox/vm_private.h>
#include <phlox/arch/vm_translation_map.h>
#include <phlox/scheduler.h>
#include <phlox/debug.h>



/* copy data to/from user space */
static status_t uspace_copy(void *usr_addr, void *kern_addr, size_t n, int to_usr)
{
    vm_address_space_t *aspace;
    vm_mapping_t *mapping;
    addr_t addr;
    status_t err;

    /* basic check of address range */
    if(!ADDR_RANGE_WITHIN_KSPACE((addr_t)kern_addr, n) ||
       !ADDR_RANGE_WITHIN_USPACE((addr_t)usr_addr, n))
        return ERR_INVALID_ARGS;

    /* current user address space */
    aspace = vm_get_current_user_aspace();
    if(aspace == NULL)
        return ERR_VM_NO_USR_ASPACE;

    /* capture the cpu to avoid thread preemption */
    sched_capture_cpu();

    /* acquire lock before touching address space */
    /*TODO: replace spinlock with different mechanism. it's really dangerous
     * to use spinlocks this way.
     */
    spin_lock(&aspace->lock);

    /* get user space mapping */
    err = vm_aspace_get_mapping(aspace, (addr_t)usr_addr, &mapping);
    if(err != NO_ERROR)
        goto err_ret_aspace;

    /* the mapping should refer to mapped memory object */
    if(mapping->type != VM_MAPPING_TYPE_OBJECT) {
        err = ERR_VM_NO_MAPPING;
        goto err_ret_aspace;
    }

    /* check that address range is within mapping */
    if(!ADDR_RANGE_WITHIN((addr_t)usr_addr, n, mapping->start,
            mapping->end - mapping->start + 1))
    {
        err = ERR_OUT_OF_RANGE;
        goto err_ret_aspace;
    }

    /* lock translation map */
    aspace->tmap.ops->lock(&aspace->tmap);

    /* check memory page permissions */
    for(addr = ROUNDOWN((addr_t)usr_addr, PAGE_SIZE);
        addr < ROUNDUP((addr_t)usr_addr+n, PAGE_SIZE);
        addr += PAGE_SIZE)
    {
        addr_t paddr;
        uint perm;
        /* query permissions for page */
        aspace->tmap.ops->query(&aspace->tmap, addr, &paddr, &perm);
        if((to_usr && !(perm & VM_PROT_WRITE)) || (!to_usr && !(perm & VM_PROT_READ)))
        {
            err = ERR_NO_PERM;
            /* unlock translation map */
            aspace->tmap.ops->unlock(&aspace->tmap);
            goto err_ret_aspace;
        }
    }

    /* unlock translation map */
    aspace->tmap.ops->unlock(&aspace->tmap);

    /* unlock before touching address space as it may cause page fault which
     * will deadlock the system.
     */
    spin_unlock(&aspace->lock);

    /* do actual data copy */
    if(to_usr)
        memcpy(usr_addr, kern_addr, n);
    else
        memcpy(kern_addr, usr_addr, n);

    /* return address space to kernel */
    vm_put_aspace(aspace);

    /* release cpu and allow preemption */
    sched_release_cpu();

    return NO_ERROR;

err_ret_aspace:
    /* return address space to kernel */
    spin_unlock(&aspace->lock);
    vm_put_aspace(aspace);
    /* unlock scheduler and allow preemption */
    sched_release_cpu();

    return err;
}

/* copy data to user space */
status_t cpy_to_uspace(void *usr_addr, const void *kern_addr, size_t n)
{
    return uspace_copy(usr_addr, (void*)kern_addr, n, 1);
}

/* copy data from user space */
status_t cpy_from_uspace(void *kern_addr, const void *usr_addr, size_t n)
{
    return uspace_copy((void*)usr_addr, kern_addr, n, 0);
}
