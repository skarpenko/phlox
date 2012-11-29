/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _KERNEL_INIT_PVT_H_
#define _KERNEL_INIT_PVT_H_

/* need for console stuff */
#define SCREEN_WIDTH  80
#define SCREEN_HEIGHT 25

/* address of bootfs image */
#define BTFS_IMAGE ((void *)0x100000)

/* MMU params */
#define DEFAULT_PAGE_FLAGS (X86_PG_ATB_P | X86_PG_ATB_W)

/* memory structure returned by int 0x15, ax 0xe820 */
struct ext_mem_struct {
    uint64 base_addr; /* Range base address */
    uint64 length;    /* Range length (in bytes) */
    uint64 type;      /* Range type:
                       *  1 - available RAM for OS usage;
                       *  2 - reserved RAM (must not be used by OS);
                       *  other - reserved for future use
                       *          (must be treated as type 2).
                       */
    uint64 filler;
} _PACKED;

extern void _start(uint32 memsize, void *ext_mem_block, uint32 ext_mem_count,
                   int in_vesa, uint32 vesa_ptr, uint32 console_ptr);
extern void clearscreen(void);
extern int puts(const char *str);
extern int dprintf(const char *fmt, ...);
extern int panic(const char *fmt, ...);

#endif
