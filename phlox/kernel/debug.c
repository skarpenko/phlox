/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <phlox/errors.h>
#include <phlox/debug.h>
#include <phlox/processor.h>
#include <phlox/thread.h>
#include <phlox/vm_page_mapper.h>
#include <phlox/klog.h>


/* need for temporary console stuff */
#define SCREEN_HEIGHT 25
#define SCREEN_WIDTH  80
#define SCREEN_PHYS   0xb8000
static unsigned short *screenBase = NULL;
static unsigned int    screenOffset = 0;
static unsigned int    line = 0;
static unsigned int    klog_ctr = 0; /* klog line counter */


/** Temporary locally used routines. Will be removed later! **/

/*
static void _clearscreen()
{
    int i;

    for(i=0; i< SCREEN_WIDTH*SCREEN_HEIGHT*2; i++) {
        screenBase[i] = 0xf20;
    }
}
*/

static void _screenup()
{
    int i;
    memcpy(screenBase, screenBase + SCREEN_WIDTH,
        SCREEN_WIDTH * SCREEN_HEIGHT * 2 - SCREEN_WIDTH * 2);
    screenOffset = (SCREEN_HEIGHT - 1) * SCREEN_WIDTH;
    for(i=0; i<SCREEN_WIDTH; i++)
        screenBase[screenOffset + i] = 0x0720;
    line = SCREEN_HEIGHT - 1;
}

static int _puts(const char *str)
{
    while (*str) {
        if (*str == '\n') {
            line++;
            if(line > SCREEN_HEIGHT - 1)
                _screenup();
            else
                screenOffset += SCREEN_WIDTH - (screenOffset % SCREEN_WIDTH);
        } else {
            screenBase[screenOffset++] = 0xf00 | *str;
        }
        if (screenOffset >= SCREEN_WIDTH * SCREEN_HEIGHT)
            _screenup();

        str++;
    }
    return 0;
}

static void _dump_klog()
{
    addr_t va;
    uint c;
    char tmp[SYSCFG_KLOG_NCOLS + 1];
    char nl[1] = "\n";

    /* set zero terminator */
    tmp[SYSCFG_KLOG_NCOLS] = 0;

    /* try to get screen page */
    if(vm_pmap_get_ppage(SCREEN_PHYS, &va, false))
        return;

    /* set screen base addres */
    screenBase = (unsigned short *)va;

    /* fetch klog rows one-by-one */
    while(klog_ctr != (c = klog_get_new_row(klog_ctr, tmp))) {
        /* put row to screen */
        _puts(tmp);
        /* if row terminated with zero put new line */
        if(!tmp[SYSCFG_KLOG_NCOLS-1])
            _puts(nl);
        klog_ctr = c;
    }

    /* put page back */
    vm_pmap_put_ppage(va);
}

/* temporary used console-writer */
static int console_writer(void *data)
{
    thread_t *me = thread_get_current_thread();
    addr_t console_addr;
    uint c;
    char tmp[SYSCFG_KLOG_NCOLS + 1];
    char nl[1] = "\n";

    /* notify */
    kprint("Console writer (id = %d): started...\n", me->id);

    /* set zero terminator */
    tmp[SYSCFG_KLOG_NCOLS] = 0;

    /* try to get screen page */
    if(vm_pmap_get_ppage(SCREEN_PHYS, &console_addr, false))
        return 0;

    /* set screen base addres */
    screenBase = (unsigned short *)console_addr;

    /* fetch klog rows one-by-one */
    while(1) {
        while(klog_ctr != (c = klog_get_new_row(klog_ctr, tmp))) {
            /* put row to screen */
            _puts(tmp);
            /* if row terminated with zero put new line */
            if(!tmp[SYSCFG_KLOG_NCOLS-1])
                _puts(nl);
            klog_ctr = c;
        }
    }
}

/* init console writer thread */
void init_console_writer(void);
void init_console_writer(void)
{
    thread_id tid;

    /* create console output thread */
    tid = thread_create_kernel_thread("kernel_console_writer",
                                      &console_writer, NULL);
    if(tid == INVALID_THREADID)
        panic("\nFailed to create console writer thread!\n");
}


/* init kernel debugging facilities */
status_t debug_init(kernel_args_t *kargs)
{
    /** will be replaced later **/
    line = kargs->cons_line;
    screenOffset = SCREEN_WIDTH * line;

    return NO_ERROR;
}

/* print into klog */
int kprint(const char *fmt, ...)
{
    int ret;
    va_list args;
    char temp[256];

    /* print into temp string */
    va_start(args, fmt);
    ret = vsnprintf(temp, 256, fmt, args);
    va_end(args);

    /* put to kernel log */
    klog_puts(temp);

    return ret;
}

/* kernel panic */
int panic(const char *fmt, ...)
{
    int ret;
    va_list args;
    char temp[256];

    /* disable interrupts */
    local_irqs_disable();

    /* print into temp string */
    va_start(args, fmt);
    ret = vsnprintf(temp, 256, fmt, args);
    va_end(args);

    /* put to kernel log */
    klog_puts("\nKERNEL PANIC: ");
    klog_puts(temp);
    klog_puts("\nsystem halted\n");

    /* Copy kernel log to screen */
    _dump_klog();

    for(;;); /* loop forever */
    /* TODO: Stop system and all CPUs (for SMP) */

    return ret;
}
