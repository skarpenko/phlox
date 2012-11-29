/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _BOOTDECOMP_H_
#define _BOOTDECOMP_H_

/* define standard routines */
#define malloc dmalloc
#define free dfree

/* kernel init entry point */
typedef void (*kinit_entry_t)(unsigned int mem, void *ext_mem_block,
                              int ext_mem_count, int in_vesa,
                              unsigned int vesa_ptr, int console_ptr);

extern void _start(unsigned int mem, void *ext_mem_block, int ext_mem_count,
                   int in_vesa, unsigned int vesa_ptr);
extern void clearscreen(void);
extern int puts(const char *str);
extern int dprintf(const char *fmt, ...);
extern void *dmalloc(unsigned int size);
extern void dfree(void *ptr);
extern int panic(const char *fmt, ...);

#endif
