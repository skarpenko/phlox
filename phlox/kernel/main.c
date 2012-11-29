/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <stdarg.h>
#include <arch/cpu.h>
#include <phlox/types.h>
#include <phlox/kernel.h>
#include <boot/bootfs.h>

#define SCREEN_HEIGHT 25
#define SCREEN_WIDTH 80

/* need for console stuff */
static unsigned short *screenBase = (unsigned short *) 0xb8000;
static unsigned int    screenOffset = 0;
static unsigned int    line = 24;

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

void _phlox_kernel_entry();  /* keep compiler happy */
void _phlox_kernel_entry() {
    asm("fninit"); /* init FPU */

    kprintf("\nWelcome to Phlox Kernel!\n");

    kprintf("\nTouching uninit data...");
    for(i=0; i<SIZE; i++) uninit[i] = 1;
    kprintf("Ok.\n");
    
    kprintf("\nTouching init data...");
    for(i=0; i<SIZE; i++) init[i] = 1;
    kprintf("Ok.\n");

    kprintf("\nRecursion test...");
    recursion(1); recursion(0);
    kprintf("Ok.\n");

    kprintf("If all tests passed - mapping is fine!\n");

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

int kprintf(const char *fmt, ...) {
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

    puts("\nKINIT PANIC: ");
    puts(temp);
    puts("\n");

    puts("system halted");
    for(;;);
    return ret;
}
