/*
* Copyright 2007-2012, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_KERNEL_H_
#define _PHLOX_KERNEL_H_

#include <stddef.h>
#include <sys/types.h>
#include <stdarg.h>
#include <arch/cpu.h>
#include <phlox/kargs.h>
#include "arch/kernel.h"

/* macro definitions */
#define ROUNDUP(a, b)   (((a) + ((b)-1)) & ~((b)-1))
#define ROUNDOWN(a, b)  (((a) / (b)) * (b))

#define MIN(a, b)  ((a) < (b) ? (a) : (b))
#define MAX(a, b)  ((a) > (b) ? (a) : (b))

#define CHECK_BIT(a, b)  ((a) & (1 << (b)))
#define SET_BIT(a, b)    ((a) | (1 << (b)))
#define CLEAR_BIT(a, b)  ((a) & (~(1 << (b))))
#define BIT(a)           (1<<a)

#define TOUCH(x) ((void)(x))
#define TOUCH_ADDR(addr) \
    { char temp = *(volatile char*)(addr); (void)temp; }

#define containerof(ptr, type, member) \
    ((type *)((addr_t)(ptr) - offsetof(type, member)))

#ifdef __cplusplus
extern "C" {
#endif

extern kernel_args_t globalKargs; /* global kernel arguments variable */

/* string formatting routines */
int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
int vscnprintf(char *buf, size_t size, const char *fmt, va_list args);
int snprintf(char *buf, size_t size, const char *fmt, ...);
int scnprintf(char *buf, size_t size, const char *fmt, ...);
int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);

/* klog output */
int kprint(const char *fmt, ...);
int panic(const char *fmt, ...);

/* duplicate string */
char *kstrdup(const char *text);

/* kernel start up stages */
enum {
    K_KERNEL_STARTUP,    /* kernel start up stage */
    K_SERVICES_STARTUP,  /* services start up */
    K_SYSTEM_READY       /* system is ready to serve */
};

/* get current kernel start up stage */
int kernel_start_stage(void);

/* returns true if specified stage is current */
bool is_kernel_start_stage(int stage);

/* returns true if specified stage was completed */
bool is_kernel_start_stage_compl(int stage);


#ifdef __cplusplus
} /* "C" */
#endif

#endif
