/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/

#ifndef _APP_SYSLIB_H_
#define _APP_SYSLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <sys/types.h>
#include <stdarg.h>


/* standard string formatting routines */
int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
int vscnprintf(char *buf, size_t size, const char *fmt, va_list args);
int snprintf(char *buf, size_t size, const char *fmt, ...);
int scnprintf(char *buf, size_t size, const char *fmt, ...);
int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);

/* print into kernel log  */
int klog_printf(const char *fmt, ...);

/*
 * Load new system service from BootFS image
 *
 * Arguments:
 *    path - path to executable.
 */
status_t service_load(const char *path);


#ifdef __cplusplus
}
#endif

#endif
