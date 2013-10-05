/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <boot/bootfs.h>
#include <phlox/errors.h>
#include <phlox/types.h>
#include <phlox/kernel.h>
#include <phlox/vm.h>
#include <phlox/vm_names.h>
#include <phlox/heap.h>
#include <phlox/process.h>
#include <phlox/thread.h>
#include <phlox/mutex.h>
#include <phlox/klog.h>
#include <phlox/debug.h>
#include <phlox/elf_file.h>
#include <phlox/imgload.h>


/* maximum number of ELF sections supported */
#define MAX_ELF_SECT 100


/* BootFS related data */
struct bootfs_data {
    const char *fname;
    bootfs_fh_t fh;
};

/* memory region obtained form ELF layout */
struct elf_region {
    unsigned start;    /* start address (at user space) */
    unsigned size;     /* size */
    unsigned flags;    /* access flags */
    addr_t vaddr;      /* currently mapped address */
    vm_object_t *obj;  /* memory object */
};

/* section header descriptor */
struct elf_shdr {
    Elf32_Shdr_t shdr;           /* section header */
    struct elf_region *region;   /* assigned region */
};


/* BootFS related data */
static addr_t   bootfs_vaddr;  /* FS virtual address */
static bootfs_t bootfs;        /* descriptor */
static mutex_t  bootfs_mutex;  /* access mutex */


/*** Internally used routines ***/

/* ELF file constructor */
static elff_err_t bootfs_elf_ctor(void *pvt)
{
    struct bootfs_data *p=(struct bootfs_data *)pvt;

    /* locate and open file */
    if(btfs_open_p(&p->fh, p->fname))
        return ELFF_USR_ERR;

    return ELFF_NO_ERR;
}

/* ELF file destructor */
static elff_err_t bootfs_elf_dtor(void *pvt)
{
    struct bootfs_data *p=(struct bootfs_data *)pvt;
    btfs_close(&p->fh);
    return ELFF_NO_ERR;
}

/* ELF file read method */
static elff_size_t bootfs_elf_read(void *pvt, elff_off_t off, elff_size_t size, void *buf)
{
    struct bootfs_data *p=(struct bootfs_data *)pvt;

    btfs_seek(&p->fh, off, SEEK_SET);

    return (elff_size_t)btfs_read(buf, 1, size, &p->fh);
}


/*** Public interface ***/


/* Init image loader */
status_t imgload_init(void)
{
    status_t err;
    object_id obj_id;

    /* locate memory object with RAM BootFS image */
    obj_id = vm_find_object_by_name(VM_NAME_BOOTFS_IMAGE);
    if(obj_id == VM_INVALID_OBJECTID)
        return ERR_NO_OBJECT;

    /* map BootFS image into kernel memory */
    err = vm_map_object(vm_get_kernel_aspace_id(), obj_id, VM_PROT_KERNEL_ALL, &bootfs_vaddr);
    if(err != NO_ERROR)
        return err;

    /* read BootFS data */
    if(btfs_mount((void*)bootfs_vaddr, &bootfs)) {
        vm_unmap_object(vm_get_kernel_aspace_id(), bootfs_vaddr);
        return ERR_INVALID_ARGS;
    }

    /* create mutex to use at operations with BootFS */
    err = mutex_create(&bootfs_mutex, NULL);
    if(err != NO_ERROR) {
        vm_unmap_object(vm_get_kernel_aspace_id(), bootfs_vaddr);
        return err;
    }

    kprint("BootFS image mapped at 0x%p\n", (void*)bootfs_vaddr);

    return NO_ERROR;
}

/* load ELF */
/* big one, refactoring needed */
status_t imgload(const char *file, uint role)
{
    unsigned i;
    /* error code */
    status_t err = NO_ERROR;
    /* BootFS data */
    struct bootfs_data dat = {
        .fname = file
    };
    /* ELF parsing variables */
    unsigned char ident[EI_NIDENT];
    Elf32_Ehdr_t elf_hdr;
    elff_obj_t elf_obj;
    struct elf_region *regions;
    struct elf_shdr   *shdrs;
    struct elf_region *region = NULL;
    unsigned n_reg = 0;
    unsigned n_sec = 0;
    /* user space process creation variables */
    process_t *proc;
    thread_id main_thread;
    aspace_id aid = vm_get_kernel_aspace_id();
    vm_address_space_t *aspace;
    object_id stack_id = VM_INVALID_OBJECTID;


    /* check that current process has permission to create processes of
     * specified role
     */
    proc = proc_get_current_process();
    i = proc->process_role;
    proc_put_process(proc);
    if(role < i)
        return ERR_NO_PERM;

    /* exclusive access to BootFS */
    mutex_lock(&bootfs_mutex);

    /* open ELF image */
    if(elff_create(&elf_obj, bootfs_elf_ctor, bootfs_elf_dtor, bootfs_elf_read, &dat) != ELFF_NO_ERR) {
        err = ERR_ILD_OPEN_FAILURE;
        goto exit_unlock_mutex;
    }

    /* read and verify identification field */
    if(elff_read_ident(&elf_obj, ident) != ELFF_NO_ERR) {
        err = ERR_ILD_INVALID_IMAGE;
        goto exit_elf_destroy;
    }
    if(elff_check_ident(ident) != ELFF_NO_ERR) {
        err = ERR_ILD_INVALID_IMAGE;
        goto exit_elf_destroy;
    }

    /* read ELF header */
    if(elff_read_ehdr32(&elf_obj, &elf_hdr) != ELFF_NO_ERR) {
        err = ERR_ILD_INVALID_IMAGE;
        goto exit_elf_destroy;
    }

    /* check that sections number is within the range */
    if(!elf_hdr.e_shnum || elf_hdr.e_shnum > MAX_ELF_SECT) {
        err = ERR_ILD_INVALID_IMAGE;
        goto exit_elf_destroy;
    }

    /* allocate space for regions */
    regions = (struct elf_region*)kmalloc(elf_hdr.e_shnum*sizeof(struct elf_region));
    if(regions == NULL) {
        err = ERR_NO_MEMORY;
        goto exit_elf_destroy;
    }

    /* allocate space for section headers */
    shdrs = (struct elf_shdr*)kmalloc(elf_hdr.e_shnum*sizeof(struct elf_shdr));
    if(shdrs == NULL) {
        kfree(regions);
        err = ERR_NO_MEMORY;
        goto exit_elf_destroy;
    }

    /*
     * read ELF section headers into memory and compute sizes of regions to
     * hold section data.
     */
    for(i=0; i < elf_hdr.e_shnum; ++i) {
        /* read section header */
        if(elff_read_shdr32(&elf_obj, &elf_hdr, i, &shdrs[n_sec].shdr) != ELFF_NO_ERR) {
            err = ERR_ILD_INVALID_IMAGE;
            goto exit_free_memory;
        }

        /* skip sections which don't hold program data */
        if(shdrs[n_sec].shdr.sh_type != SHT_PROGBITS && shdrs[n_sec].shdr.sh_type != SHT_NOBITS)
            continue;

        if(region == NULL) { /* create first region */
            region = &regions[0];
            region->start = ROUNDOWN(shdrs[n_sec].shdr.sh_addr, PAGE_SIZE);
            region->size  = ROUNDUP(shdrs[n_sec].shdr.sh_addr + shdrs[n_sec].shdr.sh_size,
                    PAGE_SIZE) - region->start;
            region->flags = shdrs[n_sec].shdr.sh_flags;
            region->vaddr = (addr_t)NULL;
            region->obj   = NULL;
        } else {
            unsigned start = ROUNDOWN(shdrs[n_sec].shdr.sh_addr, PAGE_SIZE);
            unsigned end  = ROUNDUP(shdrs[n_sec].shdr.sh_addr + shdrs[n_sec].shdr.sh_size, PAGE_SIZE);
            unsigned size  = ROUNDUP(shdrs[n_sec].shdr.sh_addr + shdrs[n_sec].shdr.sh_size,
                    PAGE_SIZE) - start;

            /* sections/regions should be in ascending order */
            if(start < region->start) {
                err = ERR_ILD_INVALID_IMAGE;
                goto exit_free_memory;
            }

            /* regions with different flags cannot overlap */
            if(start >= region->start && start < (region->start+region->size) &&
                    shdrs[n_sec].shdr.sh_flags != region->flags)
            {
                err = ERR_ILD_INVALID_IMAGE;
                goto exit_free_memory;
            }

            /*
             * if regions with same flags overlapped or close to each other then
             * merge into one region
             */
            if(start >= region->start && start <= (region->start+region->size) &&
                    shdrs[n_sec].shdr.sh_flags == region->flags)
            {
                region->size = end - region->start;
            } else { /* ... or create new region */
                ++region;

                region->start = start;
                region->size  = size;
                region->flags = shdrs[n_sec].shdr.sh_flags;
                region->vaddr = (addr_t)NULL;
                region->obj   = NULL;
            }
        }

        shdrs[n_sec].region = region;
        ++n_sec;
    }

    /* error if no regions was discovered */
    if(region == NULL) {
        err = ERR_ILD_INVALID_IMAGE;
        goto exit_free_memory;
    }
    /* number of discovered regions */
    n_reg = &region[1] - regions;


    /***** at this point we know layout of ELF in memory ******/

    /*** aid - id of kernel address space ***/

    /* create VM objects to hold ELF data. */
    for(i=0; i<n_reg; ++i) {
        /* create object */
        object_id oid = vm_create_object(NULL, regions[i].size, VM_OBJECT_PROTECT_ALL);
        if(oid == VM_INVALID_OBJECTID) {
            err = ERR_VM_GENERAL;
            goto exit_remove_objects;
        }
        /* map object into kernel space */
        err = vm_map_object(aid, oid, VM_PROT_KERNEL_ALL, &regions[i].vaddr);
        if(err != NO_ERROR)
            goto exit_remove_objects;
        /* get object structure */
        regions[i].obj = vm_get_object_by_id(oid);
    }

    /** load data into VM objects **/
    /* TODO: data of .text and .rodata sections should be mapped without copying */
    for(i=0; i<n_sec; ++i) {
        unsigned long offs = shdrs[i].shdr.sh_addr - shdrs[i].region->start;

        /* load data of program data section into memory */
        if(shdrs[i].shdr.sh_type == SHT_PROGBITS) {
                if(elff_read_sdata32(&elf_obj, &shdrs[i].shdr, 0, shdrs[i].shdr.sh_size,
                        &((unsigned char *)shdrs[i].region->vaddr)[offs]) != shdrs[i].shdr.sh_size)
                {
                    err = ERR_ILD_INVALID_IMAGE;
                    goto exit_remove_objects;
                }
        } else if(shdrs[i].shdr.sh_type == SHT_NOBITS) {
             /* this branch not needed as all pages assumed to be cleared by
              * pages allocator.
              */
             /* memset(&((unsigned char *)shdrs[i].region->vaddr)[offs], 0, shdrs[i].shdr.sh_size); */
        }
    }


    /** user space process creation starts here **/

    /* create user address space */
    aid = vm_create_aspace(NULL, USER_BASE, USER_SIZE);
    if(aid == VM_INVALID_ASPACEID) {
        err = ERR_VM_GENERAL;
        goto exit_remove_objects;
    }

    /*** aid - user address space id at this point */

    /** unmap objects from kernel space **/
    for(i=0; i<n_reg; ++i) {
        vm_unmap_object(vm_get_kernel_aspace_id(), regions[i].vaddr);
        regions[i].vaddr = (addr_t)NULL;
    }

    /** map objects into user space **/
    for(i=0; i<n_reg; ++i) {
        /* all regions are available for read by default */
        unsigned o_prot = VM_OBJECT_PROTECT_READ;
        unsigned m_prot = VM_PROT_USER_READ;

        /* check if write and execute is also possible */
        o_prot |= ((regions[i].flags & SHF_WRITE) ? VM_OBJECT_PROTECT_WRITE : 0);
        m_prot |= ((regions[i].flags & SHF_WRITE) ? VM_PROT_USER_WRITE : 0);

        o_prot |= ((regions[i].flags & SHF_EXECINSTR) ? VM_OBJECT_PROTECT_EXECUTE : 0);
        m_prot |= ((regions[i].flags & SHF_EXECINSTR) ? VM_PROT_USER_EXECUTE : 0);

        regions[i].obj->protect = o_prot;

        /* map object exactly to specified address with appropriate permissions */
        err = vm_map_object_exactly(aid, regions[i].obj->id, m_prot, regions[i].start);
        if(err != NO_ERROR)
            goto exit_remove_objects;
    }

    /* create memory object for user thread stack */
    stack_id = vm_create_object(NULL, USER_STACK_SIZE*PAGE_SIZE,
                VM_OBJECT_PROTECT_READ | VM_OBJECT_PROTECT_WRITE);
    if(stack_id == VM_INVALID_OBJECTID) {
        err = ERR_VM_GENERAL;
        goto exit_remove_objects;
    }

    /* create user process */
    aspace = vm_get_aspace_by_id(aid);
    proc = proc_create_user_process(file, proc_get_current_process(), aspace, NULL, role);
    if(!proc) {
        vm_put_aspace(aspace);
        err = ERR_MT_GENERAL;
        goto exit_remove_objects;
    }

    /* return address space */
    vm_put_aspace(aspace);

    /* create main user thread */
    main_thread = thread_create_user_thread(NULL, proc, elf_hdr.e_entry, NULL, stack_id, 0, false);
    if(main_thread == INVALID_THREADID) {
        proc_destroy_process(proc->id);
        goto exit_remove_objects;
    }

    /** at this point user code may be scheduled for execution or already executed **/

    /* return process structure to the system */
    proc_put_process(proc);

    /* free resources before exit */
    goto exit_free_memory;
/*******************************/

/* unmap and remove created memory objects */
exit_remove_objects:
    for(i=0; i<n_reg; ++i) {
        if(regions[i].vaddr)
            vm_unmap_object(aid, regions[i].vaddr);

        if(regions[i].obj) {
            vm_delete_object(regions[i].obj->id);
            vm_put_object(regions[i].obj);
        }
    }

    /* delete user address space */
    if(aid != vm_get_kernel_aspace_id())
        vm_delete_aspace(aid);

/* free memory allocated for temporary structures */
exit_free_memory:
    kfree(regions);
    kfree(shdrs);

/* destroy ELF file reader */
exit_elf_destroy:
    elff_destroy(&elf_obj);

/* release mutex */
exit_unlock_mutex:
    mutex_unlock(&bootfs_mutex);

    return err;
}
