/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_I386_USTUB_TABLE_H_
#define _PHLOX_ARCH_I386_USTUB_TABLE_H_

#include <arch/cpu.h>
#include <phlox/arch/i386/kernel.h>


/* User space stub table start / end */
extern unsigned char user_stub_table_start[];
extern unsigned char user_stub_table_end[];

/* Address of table mapping at user space - last page */
#define USER_STUB_TABLE_BASE  (USER_BASE+USER_SIZE-PAGE_SIZE)


/** Stubs entry points **/

/* Thread exit */
#define USER_STUB_THREAD_EXIT  (USER_STUB_TABLE_BASE + 0x000)


#endif
