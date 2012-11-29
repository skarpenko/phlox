/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_I386_CPU_H
#define _PHLOX_ARCH_I386_CPU_H

/* Memory page size */
#define PAGE_SIZE 4096

/* Max. limit of Global Descriptor Table */
#define MAX_GDT_LIMIT  0x800
/*
* NOTE: Global Descroptor Table limit can be up to 0x10000 bytes,
* but Phlox uses only 0x800 (256 entries).
*/

/* Max. limit of Interrupt Descriptor Table */
#define MAX_IDT_LIMIT  0x800

#endif
