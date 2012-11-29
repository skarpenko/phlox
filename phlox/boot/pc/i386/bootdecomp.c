/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <stdarg.h>
#include <arch/cpu.h>
#include <phlox/kernel.h>
#include <boot/bootfs.h>
#include "inflate.h"
#include "bootdecomp.h"

/* need for console stuff */
#define SCREEN_WIDTH  80
#define SCREEN_HEIGHT 25
static unsigned short *screenBase = (unsigned short *) 0xb8000;
static unsigned int    screenOffset = 0;
static unsigned int    line = 0;


static unsigned char *heap_ptr = (unsigned char *)0x400000;

#define TARGET ((void *)0x100000)
extern void *_compressed_image;

void _start(unsigned int mem, void *ext_mem_block, int ext_mem_count, int in_vesa, unsigned int vesa_ptr) {
    unsigned long len;
    bootfs_t btfs;
    btfs_dir_entry *en;
    kinit_entry_t kinit;

    clearscreen();

    dprintf("Phlox boot, decompressing system.");

    len = gunzip((unsigned char const *)&_compressed_image, TARGET, dmalloc(32*1024));
    dprintf("done, len %d\n", len);

    dprintf("mounting bootfs at %p\n", TARGET);
    if( btfs_mount(TARGET, &btfs) )
       panic("error mounting bootfs.\n");

    dprintf("starting kinit...");
    en = btfs_locate("kinit");
    if(!en)
      panic("kinit not found.\n");
    kinit = (void *)((char *)btfs.addr + en->offset*PAGE_SIZE + en->code_ventr);
    dprintf("entry at %p\n", kinit);

    /* jump into kinit */
    kinit(mem, ext_mem_block, ext_mem_count, in_vesa, vesa_ptr, screenOffset);
}

void *dmalloc(unsigned int size) {
    return (heap_ptr -= size);
}

void dfree(void *ptr) {
}

void clearscreen() {
    int i;

    for(i=0; i< SCREEN_WIDTH*SCREEN_HEIGHT*2; i++) {
        screenBase[i] = 0xf20;
    }
}

static void screenup()
{
    int i;
    memcpy(screenBase, screenBase + SCREEN_WIDTH,
        SCREEN_WIDTH * SCREEN_HEIGHT * 2 - SCREEN_WIDTH * 2);
    screenOffset = (SCREEN_HEIGHT - 1) * SCREEN_WIDTH;
    for(i=0; i<SCREEN_WIDTH; i++)
        screenBase[screenOffset + i] = 0x0720;
    line = SCREEN_HEIGHT - 1;
}

int puts(const char *str) {
    while (*str) {
        if (*str == '\n') {
            line++;
            if(line > SCREEN_HEIGHT - 1)
                screenup();
            else
                screenOffset += SCREEN_WIDTH - (screenOffset % SCREEN_WIDTH);
        } else {
            screenBase[screenOffset++] = 0xf00 | *str;
        }
        if (screenOffset >= SCREEN_WIDTH * SCREEN_HEIGHT)
            screenup();

        str++;
    }
    return 0;
}

int dprintf(const char *fmt, ...) {
    int ret;
    va_list args;
    char temp[256];

    va_start(args, fmt);
    ret = vsprintf(temp,fmt,args);
    va_end(args);

    puts(temp);
    return ret;
}

int panic(const char *fmt, ...) {
    int ret;
    va_list args;
    char temp[256];

    va_start(args, fmt);
    ret = vsprintf(temp,fmt,args);
    va_end(args);

    puts("\nPANIC: ");
    puts(temp);
    puts("\n");

    puts("system halted");
    for(;;);
    return ret;
}
