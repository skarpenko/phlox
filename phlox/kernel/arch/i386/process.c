/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/errors.h>
#include <phlox/arch/i386/ustub_table.h>
#include <phlox/vm.h>
#include <phlox/process.h>
#include <phlox/thread_private.h>



/* VM object with user space stub table */
static object_id usr_stub_id;


/* architecture-specific process module init */
status_t arch_process_init(kernel_args_t *kargs)
{
    status_t err;
    addr_t vaddr;
    unsigned char *p, *dst;
    vm_object_t *obj;

    /* create memory object for user stub table code */
    usr_stub_id = vm_create_object("user_stub_table", PAGE_SIZE, VM_OBJECT_PROTECT_ALL);
    if(usr_stub_id == VM_INVALID_OBJECTID)
        return ERR_VM_GENERAL;

    /* map stub table memory to kernel space */
    err = vm_map_object(vm_get_kernel_aspace_id(), usr_stub_id, VM_PROT_KERNEL_ALL, &vaddr);
    if(err) {
        vm_delete_object(usr_stub_id); /* actually, recovery is not expected */
        return err;
    }

    /* copy stub table code to mapped memory object */
    dst = (unsigned char *)vaddr;
    for(p = user_stub_table_start; p != user_stub_table_end; ++p, ++dst)
       *dst = *p;

    /*
     * TODO: think on better solution than copying. Optionally, kernel image
     * VM object could be separated into several and one could be this table.
     */

    /* unmap object from kernel space */
    err = vm_unmap_object(vm_get_kernel_aspace_id(), vaddr);
    if(err) {
        vm_delete_object(usr_stub_id);
        return err;
    }

    /* change object access rights to read and execute */
    obj = vm_get_object_by_id(usr_stub_id);
    obj->protect = VM_OBJECT_PROTECT_READ | VM_OBJECT_PROTECT_EXECUTE;
    vm_put_object(obj);

    /* nothing init for now */
    return NO_ERROR;
}

/* architecture-specific process module per CPU init */
status_t arch_process_init_per_cpu(kernel_args_t *kargs, uint curr_cpu)
{
    /* nothing to init */

    return NO_ERROR;
}

/* init arch-specific fields of process structure */
status_t arch_init_process_struct(process_t *proc)
{
    /* not for kernel process */
    if(proc->aid == vm_get_kernel_aspace_id())
        return NO_ERROR;

    /* map stub table to user space */
    return vm_map_object_exactly(proc->aid, usr_stub_id,
            VM_PROT_USER_READ | VM_PROT_USER_EXECUTE, USER_STUB_TABLE_BASE);
}
