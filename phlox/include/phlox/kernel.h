/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_KERNEL_H_
#define _PHLOX_KERNEL_H_

#include <stddef.h>
#include <sys/types.h>
#include <stdarg.h>
#include <arch/cpu.h>
#include "arch/kernel.h"

/* macro definitions */
#define ROUNDUP(a, b)   (((a) + ((b)-1)) & ~((b)-1))
#define ROUNDOWN(a, b)  (((a) / (b)) * (b))

/* to align the pointer to the (next) page boundary (same as ROUNDUP) */
#define PAGE_ALIGN(a)   (((a)+PAGE_SIZE-1)&PAGE_MASK)

#define min(a, b)  ((a) < (b) ? (a) : (b))
#define max(a, b)  ((a) > (b) ? (a) : (b))

#define CHECK_BIT(a, b)  ((a) & (1 << (b)))
#define SET_BIT(a, b)    ((a) | (1 << (b)))
#define CLEAR_BIT(a, b)  ((a) & (~(1 << (b))))

#define TOUCH(x) ((void)(x))

#ifdef __cplusplus
extern "C" {
#endif

int vsprintf(char *str, char const *format, va_list ap);
int sprintf(char * buf, const char *fmt, ...);

#ifdef __cplusplus
} /* "C" */
#endif

#endif
