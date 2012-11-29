/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/mutex.h>

#warning "MUTEXES IS NOT IMPLEMENTED!!!"

uint32 mutex_init(mutex_t *mtx, const char *in_name) {
    kprint("mutex_init: not implemented!\n");

    return 0;
}

void mutex_destroy(mutex_t *mtx) {
    /* not implemented */
}

void mutex_lock(mutex_t *mtx) {
    /* not implemented */
}

void mutex_unlock(mutex_t *mtx) {
    /* not implemented */
}
