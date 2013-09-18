/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_IMGLOAD_H
#define _PHLOX_IMGLOAD_H


/*
 * Init image loader
 * Called once at startup
 */
status_t imgload_init(void);


/*
 * Load ELF from BootFS and create user-space process
 */
status_t imgload(const char *file, uint role);


#endif
