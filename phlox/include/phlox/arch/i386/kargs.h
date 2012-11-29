/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_I386_KERNEL_ARGS_H_
#define _PHLOX_ARCH_I386_KERNEL_ARGS_H_

#define MAX_BOOT_PTABLES 4

/* architecture specific kernel args */
typedef struct {
    /* architecture specific */
    uint32 phys_pgdir;    /* physical address of page directory */
    uint32 virt_pgdir;    /* virtual address of page directory */
    uint32 num_pgtables;  /* number of allocated page tables */
    uint32 phys_pgtables[MAX_BOOT_PTABLES]; /* physical addresses of page tables */
    uint32 virt_pgtables[MAX_BOOT_PTABLES]; /* virtual addresses of page tables */

    /* SMP stuff (currently not used) */
    uint32 apic_time_cv_factor; /* apic ticks per second */
    uint32 apic_phys;
    uint32 *apic;
    uint32 ioapic_phys;
    uint32 *ioapic;
    uint32 cpu_apic_id[SYSCFG_MAX_CPUS];
    uint32 cpu_os_id[SYSCFG_MAX_CPUS];
    uint32 cpu_apic_version[SYSCFG_MAX_CPUS];
} arch_kernel_args_t;

#endif
