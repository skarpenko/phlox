/*
* Copyright 2007-2011, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_SEM_H
#define _PHLOX_SEM_H

#include <phlox/types.h>
#include <phlox/ktypes.h>
#include <phlox/kargs.h>


/* Reserved ID */
#define INVALID_SEMID ((sem_id)0)  /* Invalid semaphore ID */


/*
 * Called only at system startup for semaphores initialization.
*/
status_t semaphores_init(kernel_args_t *kargs);


#endif
