/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_VM_NAMES_H_
#define _PHLOX_VM_NAMES_H_

#include <phlox/arch/vm_names.h>
#include <phlox/platform/vm_names.h>


/* Name of kernel image object */
#define VM_NAME_KERNEL_IMAGE           "kernel_image"
/* Name format for per CPU stack object */
#define VM_NAME_KERNEL_CPU_STACK_FMT   "kernel_cpu%d_stack"
/* Name of kernel heap object */
#define VM_NAME_KERNEL_HEAP            "kernel_heap"
/* Name of physical pages descriptors array object */
#define VM_NAME_KERNEL_PHYSICAL_PAGES  "kernel_physical_pages"
/* Name of mapping descriptors array object */
#define VM_NAME_KERNEL_MAPPING_DESCR   "kernel_mapping_descr"
/* Name of BootFS image object */
#define VM_NAME_BOOTFS_IMAGE           "bootfs_image"


#endif
