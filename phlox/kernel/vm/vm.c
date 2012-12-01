/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <phlox/errors.h>
#include <phlox/spinlock.h>
#include <phlox/vm_private.h>
#include <phlox/vm_page.h>
#include <phlox/vm.h>
#include <phlox/arch/vm_translation_map.h>
#include <phlox/heap.h>


/* Global variable with Virtual Memory State */
vm_stat_t VM_State;

/* software page fault handler */
static status_t vm_soft_page_fault(addr_t addr, bool is_write, bool is_exec, bool is_user);


/* init virtual memory */
status_t vm_init(kernel_args_t *kargs)
{
    addr_t heap_base;  /* Kernel's heap base */
    size_t heap_size;  /* Kernel's heap size */
    status_t err;

    /* clear VM statistics */
    memset(&VM_State, 0, sizeof(vm_stat_t));

    /* translation map module init */
    err = vm_translation_map_init(kargs);
    if(err)
       panic("vm_init: translation map init failed!\n");

    /* execute architecture-specific init */
    err = arch_vm_init(kargs);
    if(err)
       panic("arch_vm_init: failed!\n");

    /* start platform-specific init */
    err = platform_vm_init(kargs);
    if(err)
       panic("platform_vm_init: failed!\n");

    /* init memory size */
    err = vm_page_preinit(kargs);
    if(err)
       panic("vm_page_preinit: failed!\n");

    /* init kernel's heap */
    {
        /* compute heap size */
        heap_size = ROUNDUP(
                      vm_phys_mem_size() / SYSCFG_KERNEL_HEAP_FRAC,
                      1*1024*1024 /* Mbyte */
                    );
        if(heap_size > SYSCFG_KERNEL_HEAP_MAX)
              heap_size = SYSCFG_KERNEL_HEAP_MAX;

        /* allocate heap area */
        heap_base = vm_alloc_from_kargs(kargs, heap_size, VM_PROT_KERNEL_DEFAULT);

        /* init heap */
        heap_init(heap_base, heap_size);
        /* Fuf... Now kmalloc and kfree is available */
    }

    /* init vm page module */
    err = vm_page_init(kargs);
    if(err)
       panic("vm_page_init: failed!\n");

/*** Important note: After this point vm_alloc_from_kargs must not be used
 *** because physical pages bookkeping is turned on.
 ***/
 
    /* prefinal stage of translation map module init */
    err = vm_translation_map_init_prefinal(kargs);
    if(err)
       panic("vm_init: prefinal stage of translation map init failed!\n");

    /* start prefinal init stage of architecture-specific parts */
    err = arch_vm_init_prefinal(kargs);
    if(err)
       panic("arch_vm_init_prefinal: stage failed!\n");

    /* start prefinal stages of platform-specific init */
    err = platform_vm_init_prefinal(kargs);
    if(err)
       panic("platform_vm_init_prefinal: stage failed!\n");

    /* init address spaces module */
    err = vm_address_spaces_init(kargs);
    if(err)
       panic("vm_address_spaces_init: failed!\n");

    /* init vm objects module */
    err = vm_objects_init(kargs);
    if(err)
       panic("vm_objects_init: failed!\n");

    /* create initial kernel space */
    if(vm_create_kernel_aspace("kernel_space", KERNEL_BASE, KERNEL_SIZE) == VM_INVALID_ASPACEID)
       panic("vm_init: failed to create initial kernel space!\n");

    /*** Final stages of VM init ***/

    /* final stage of translation map module init */
    err = vm_translation_map_init_final(kargs);
    if(err)
       panic("vm_init: final stage of translation map init failed!\n");

    /* start final init stage of architecture-specific parts */
    err = arch_vm_init_final(kargs);
    if(err)
       panic("arch_vm_init_final: final stage failed!\n");

    /* final stages of platform-specific init */
    err = platform_vm_init_final(kargs);
    if(err)
       panic("platform_vm_init_final: final stage failed!\n");

    /* final stages of vm page module init */
    err = vm_page_init_final(kargs);
    if(err)
       panic("vm_page_init_final: failed!\n");

    /* init memory structures */
    {
#       define _NAME_LEN  30
        aspace_id kid = vm_get_kernel_aspace_id();
        object_id id;
        char name[_NAME_LEN];
        int i;

        /* create kernel_image memory object and its mapping */
        id = vm_create_physmem_object("kernel_image", kargs->phys_kernel_addr.start,
                                      kargs->phys_kernel_addr.size, VM_OBJECT_PROTECT_ALL);
        if(id == VM_INVALID_OBJECTID)
            panic("vm_init: failed to create kernel_image object!\n");

        err = vm_map_object_exactly(kid, id, VM_PROT_KERNEL_ALL, kargs->virt_kernel_addr.start);
        if(err != NO_ERROR)
            panic("vm_init: failed to create mapping of kernel_image object!\n");

        /* init physical page counters */
        err = vm_page_init_wire_counters(kargs->virt_kernel_addr.start,
                                         kargs->virt_kernel_addr.size);
        if(err != NO_ERROR)
            panic("vm_init: failed to init counters for kernel_image object!\n");

        /* create kernel stacks memory objects and mappings */
        for(i = 0; i < SYSCFG_MAX_CPUS; i++) {
            snprintf(name, _NAME_LEN, "kernel_cpu%d_stack", i);
            id = vm_create_physmem_object(name, kargs->phys_cpu_kstack[i].start,
                                          kargs->phys_cpu_kstack[i].size, VM_OBJECT_PROTECT_ALL);
            if(id == VM_INVALID_OBJECTID)
                panic("vm_init: failed to create %s object!\n", name);

            err = vm_map_object_exactly(kid, id, VM_PROT_KERNEL_ALL, kargs->virt_cpu_kstack[i].start);
            if(err != NO_ERROR)
                panic("vm_init: failed to create mapping of %s object!\n", name);

            /* init counters */
            err = vm_page_init_wire_counters(kargs->virt_cpu_kstack[i].start,
                                             kargs->virt_cpu_kstack[i].size);
            if(err != NO_ERROR)
                panic("vm_init: failed to init counters for %s object!\n", name);
        }

        /* create kernel_heap memory object and its mapping */
        id = vm_create_virtmem_object("kernel_heap", kid, heap_base, heap_size, VM_OBJECT_PROTECT_ALL);
        if(id == VM_INVALID_OBJECTID)
            panic("vm_init: failed to create kernel_heap object!\n");

        err = vm_map_object_exactly(kid, id, VM_PROT_KERNEL_ALL, heap_base);
        if(err != NO_ERROR)
            panic("vm_init: failed to create mapping of kernel_heap object!\n");

        /* init counters for heap pages */
        err = vm_page_init_wire_counters(heap_base, heap_size);
        if(err != NO_ERROR)
            panic("vm_init: failed to init counters for kernel_heap object!\n");

        /* create bootfs_image memory object */
        id = vm_create_physmem_object("bootfs_image", kargs->btfs_image_addr.start,
                                      kargs->btfs_image_addr.size, VM_OBJECT_PROTECT_ALL);
        if(id == VM_INVALID_OBJECTID)
            panic("vm_init: failed to create bootfs_image object!\n");

#       undef _NAME_LEN
    }
    /* end of init memory structures */

    return NO_ERROR;
}

/* allocate virtual space from kernel args */
addr_t vm_alloc_vspace_from_kargs(kernel_args_t *kargs, size_t size)
{
    addr_t spot = 0;
    uint i;
    int last_valloc_entry = 0;

    size = PAGE_ALIGN(size);
    
    /* find a slot in the virtual allocation addr_t range */
    for(i=1; i < kargs->num_virt_alloc_ranges; i++) {
        last_valloc_entry = i;
        /* check to see if the space between this one and the last is big enough */
        if(kargs->virt_alloc_range[i].start -
          (kargs->virt_alloc_range[i-1].start +
           kargs->virt_alloc_range[i-1].size) >= size) {
             spot = kargs->virt_alloc_range[i-1].start +
                     kargs->virt_alloc_range[i-1].size;
             kargs->virt_alloc_range[i-1].size += size;
             goto out;
        }
    }
    if(spot == 0) {
        /* we hadn't found one between allocation ranges. this is ok.
         * see if there's a gap after the last one
         */
        if(kargs->virt_alloc_range[last_valloc_entry].start +
           kargs->virt_alloc_range[last_valloc_entry].size + size <=
              KERNEL_BASE + (KERNEL_SIZE - 1)) {
              spot = kargs->virt_alloc_range[last_valloc_entry].start +
                      kargs->virt_alloc_range[last_valloc_entry].size;
              kargs->virt_alloc_range[last_valloc_entry].size += size;
              goto out;
        }
        /* see if there's a gap before the first one */
        if(kargs->virt_alloc_range[0].start > KERNEL_BASE) {
            if(kargs->virt_alloc_range[0].start - KERNEL_BASE >= size) {
                kargs->virt_alloc_range[0].start -= size;
                spot = kargs->virt_alloc_range[0].start;
                goto out;
            }
        }
    }

    out: return spot;
}

/* allocates memory block of given size form kernel args */
addr_t vm_alloc_from_kargs(kernel_args_t *kargs, size_t size, uint protection)
{
    addr_t vspot;
    addr_t pspot;
    uint i;

    /* find the vaddr to allocate at */
    vspot = vm_alloc_vspace_from_kargs(kargs, size);

    /* map the pages */
    for(i=0; i < PAGE_ALIGN(size)/PAGE_SIZE; i++) {
        pspot = vm_alloc_ppage_from_kargs(kargs);
        if(pspot == 0)
            panic("error allocating physical page from globalKargs!\n");
        vm_tmap_quick_map_page(kargs, vspot + i*PAGE_SIZE, pspot * PAGE_SIZE, protection);
    }

    /* return start address of allocated block */
    return vspot;
}

/* return available physical memory size */
size_t vm_phys_mem_size(void)
{
    return VM_State.total_physical_pages * VM_State.physical_page_size;
}

/* software page fault handler */
static status_t vm_soft_page_fault(addr_t addr, bool is_write, bool is_exec, bool is_user)
{
    return NO_ERROR;
}

/* hardware page fault handler. called on page fault exception. */
result_t vm_hard_page_fault(addr_t addr, addr_t fault_addr, bool is_write, bool is_exec, bool is_user)
{
    status_t err;

    err = vm_soft_page_fault(addr, is_write, is_exec, is_user);
    if(err != NO_ERROR)
       panic("vm_hard_page_fault(): can't handle!");

    return NO_ERROR;
}

/* map object */
status_t vm_map_object(aspace_id aid, object_id oid, uint protection, addr_t *vaddr)
{
    vm_address_space_t *aspace;
    vm_object_t *object;
    vm_mapping_t *mapping;
    status_t err;

    /* get address space */
    aspace = vm_get_aspace_by_id(aid);
    if(aspace == NULL)
        return ERR_VM_INVALID_ASPACE;

    /* get memory object */
    object = vm_get_object_by_id(oid);
    if(object == NULL) {
        vm_put_aspace(aspace);
        return ERR_VM_INVALID_OBJECT;
    }

    /* acquire locks.
     * WARNING: If interrupts disabled this can lead into deadlock!
     *          Spinlocks must be replaced with semaphores or mutexes!
     */
    spin_lock(&aspace->lock);
    spin_lock(&object->lock);
#warning "vm_map_object(): Dangerous spinlocks sequence!"

    /* create mapping */
    err = vm_aspace_create_mapping(aspace, object->size, &mapping);
    if(err != NO_ERROR)
        goto exit_map;

    /* stick object to mapping */
    mapping->type = VM_MAPPING_TYPE_OBJECT;
    mapping->object = object;

    /* store new mapping in mappings list of the object */
    vm_object_put_mapping(object, mapping);

    /* unlock object */
    spin_unlock(&object->lock);

    object = NULL;

    /* return start address where object was mapped */
    *vaddr = mapping->start;

exit_map:
    /* release locks */
    if(object != NULL)
        spin_unlock(&object->lock);
    spin_unlock(&aspace->lock);

    /* put structures back */
    if(object != NULL)
        vm_put_object(object);
    /* NOTE: If object was mapped successfully it is not needed to put it */
    vm_put_aspace(aspace);

    return err;
}

/* map object exactly at provided virtual address */
status_t vm_map_object_exactly(aspace_id aid, object_id oid, uint protection, addr_t vaddr)
{
    vm_address_space_t *aspace;
    vm_object_t *object;
    vm_mapping_t *mapping;
    status_t err;

    /* get address space */
    aspace = vm_get_aspace_by_id(aid);
    if(aspace == NULL)
        return ERR_VM_INVALID_ASPACE;

    /* get memory object */
    object = vm_get_object_by_id(oid);
    if(object == NULL) {
        vm_put_aspace(aspace);
        return ERR_VM_INVALID_OBJECT;
    }

    /* acquire locks.
     * WARNING: If interrupts disabled this can lead into deadlock!
     *          Spinlocks must be replaced with semaphores or mutexes!
     */
    spin_lock(&aspace->lock);
    spin_lock(&object->lock);
#warning "vm_map_object_exactly(): Dangerous spinlocks sequence!"

    /* create mapping */
    err = vm_aspace_create_mapping_exactly(aspace, vaddr, object->size, &mapping);
    if(err != NO_ERROR)
        goto exit_map;

    /* ... and assign object to it */
    mapping->type = VM_MAPPING_TYPE_OBJECT;
    mapping->object = object;

    /* store new mapping in object mappings list */
    vm_object_put_mapping(object, mapping);

    /* unlock object */
    spin_unlock(&object->lock);

    object = NULL;

exit_map:
    /* release locks */
    if(object != NULL)
        spin_unlock(&object->lock);
    spin_unlock(&aspace->lock);

    /* put structures back */
    if(object != NULL)
        vm_put_object(object);
    vm_put_aspace(aspace);

    return err;
}

/* unmap object that is mapped at given virtual address */
status_t vm_unmap_object(aspace_id aid, addr_t vaddr)
{
    vm_address_space_t *aspace;
    vm_mapping_t *mapping;
    status_t err;

    /* get mapping */
    aspace = vm_get_aspace_by_id(aid);
    if(aspace == NULL)
        return ERR_VM_INVALID_ASPACE;

    /* acquire lock */
    spin_lock(&aspace->lock);

    /* get mapping at provided virtual address */
    err = vm_aspace_get_mapping(aspace, vaddr, &mapping);
    if(err != NO_ERROR)
        goto exit_unmap;

    /* check mapping type */
    if(mapping->type != VM_MAPPING_TYPE_OBJECT) {
        err = ERR_VM_BAD_ADDRESS;
        goto exit_unmap;
    }

    /* acquire object lock */
    spin_lock(&mapping->object->lock);

    /* remove mapping from object mappings list */
    vm_object_remove_mapping(mapping->object, mapping);

    /* release lock */
    spin_unlock(&mapping->object->lock);

    /* ... and put object back */
    vm_put_object(mapping->object);

    /* delete mapping from address space */
    vm_aspace_delete_mapping(aspace, mapping);

exit_unmap:
    /* unlock address space */
    spin_unlock(&aspace->lock);

    /* ... and but it back */
    vm_put_aspace(aspace);

    return err;
}
