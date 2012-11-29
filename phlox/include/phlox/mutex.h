/*
* Copyright 2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_MUTEX_H
#define _PHLOX_MUTEX_H

#include <phlox/kernel.h>

typedef struct {
    thread_id holder;
    sem_id sem;
} mutex_t;

uint32 mutex_init(mutex_t *mtx, const char *name);
void mutex_destroy(mutex_t *mtx);
void mutex_lock(mutex_t *mtx);
void mutex_unlock(mutex_t *mtx);


#endif
