/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <stdarg.h>
#include <arch/cpu.h>
#include <phlox/types.h>
#include <phlox/kernel.h>
#include <phlox/processor.h>
#include <phlox/kargs.h>
#include <boot/bootfs.h>

#define SCREEN_HEIGHT 25
#define SCREEN_WIDTH 80
#define kprintf kprint

/* Global kernel args */
kernel_args_t globalKargs;

/* need for console stuff */
static unsigned short *screenBase = (unsigned short *) 0xb8000;
static unsigned int    screenOffset = 0;
static unsigned int    line = 0;

/* temporary for tests */
#define SIZE 500000
#define R 200
char uninit[SIZE];
char init[SIZE] = { 0 };
uint32 i;
uint32 rc;

static void recursion(uint32 init);
static void recursion(uint32 init) {
  if(init) { rc = 0; return; }

  if(rc>=R) return;

  rc++;
  recursion(0);
}
/**end of temporary**/

void _phlox_kernel_entry(kernel_args_t *kargs, uint32 num_cpu);  /* keep compiler happy */
void _phlox_kernel_entry(kernel_args_t *kargs, uint32 num_cpu) {
    /* if we are bootstrap processor,
     *  store kernel args to global variable.
     */
    if(num_cpu==0)
      memcpy(&globalKargs, kargs, sizeof(kernel_args_t));

    /** will be replaced later **/
    line = kargs->cons_line;
    screenOffset = SCREEN_WIDTH * line;
    kprintf("\nWelcome to Phlox Kernel!\n");

    /* processor set initialization */
    processor_set_init(&globalKargs, num_cpu);

    kprintf("Kernel Args test...");
    kprintf("btfs_base = %x, size = %d\n", kargs->btfs_image_addr.start,
                                           kargs->btfs_image_addr.size);

    kprintf("Touching uninit data...");
    for(i=0; i<SIZE; i++) uninit[i] = 1;
    kprintf("Ok.\n");
    
    kprintf("Touching init data...");
    for(i=0; i<SIZE; i++) init[i] = 1;
    kprintf("Ok.\n");

    kprintf("Recursion test...");
    recursion(1); recursion(0);
    kprintf("Ok.\n");

    kprintf("\nIf all tests passed - mapping is fine!\n");

    kprintf("\n\nexecuting infinite loop.\n");

    for(;;);
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

int kprint(const char *fmt, ...) {
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

    puts("\nKERNEL PANIC: ");
    puts(temp);
    puts("\n");

    puts("system halted");
    for(;;);
    return ret;
}
