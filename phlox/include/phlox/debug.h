/*
* Copyright 2007-2012, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_DEBUG_H
#define _PHLOX_DEBUG_H

#include <sys/debug.h>
#include <phlox/kernel.h>
#include <phlox/kargs.h>


/*
 * Init kernel debugging facilities.
 * Called during system boot up.
 */
status_t debug_init(kernel_args_t *kargs);


/*
 * Init console output
 *
 * Called during system boot after VM init, mainly for mapping
 * of screen buffer into kernel memory.
 */
status_t debug_init_console_output(kernel_args_t *kargs);


/*
 * Init console writer thread
 */
void debug_init_console_writer(void);


#endif
