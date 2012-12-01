/*
* Copyright 2007-2012, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_MUTEX_H
#define _PHLOX_MUTEX_H

#include <phlox/types.h>


/* Mutex type */
typedef struct {
    thread_id holder;  /* Mutex holding thread */
    sem_id    sem;     /* Underlying semaphore */
} mutex_t;


/*
 * Init mutex structure
*/
void mutex_init(mutex_t *mtx);

/*
 * Create mutex
 *
 * Name will be assigned to underlying semaphore,
 * it can be NULL.
*/
status_t mutex_create(mutex_t *mtx, const char *name);

/*
 * Destroy mutex
*/
status_t mutex_destroy(mutex_t *mtx);

/*
 * Acquire mutex
*/
status_t mutex_lock(mutex_t *mtx);

/*
 * Release mutex
*/
status_t mutex_unlock(mutex_t *mtx);


#endif
