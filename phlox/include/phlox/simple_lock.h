/*
* Copyright 2007-2011, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_SIMPLE_LOCK_H
#define _PHLOX_SIMPLE_LOCK_H

#include <phlox/spinlock.h>
#include <phlox/thread.h>


/* Simple lock type */
typedef spinlock_t simple_lock_t;


/* Init simple lock in unlocked state */
static inline void simple_lock_init(simple_lock_t *lock)
{
    spin_init(lock);
}

/* Init simple lock in locked state */
static inline void simple_lock_init_locked(simple_lock_t *lock)
{
    spin_init_locked(lock);
}

/* Acquire simple lock */
static inline void simple_lock(simple_lock_t *lock)
{
    while(!spin_trylock(lock))
        thread_yield();
}

/* Release simple lock */
static inline void simple_unlock(simple_lock_t *lock)
{
    spin_unlock(lock);
}

/* Try to acquire simple lock */
static inline int simple_trylock(simple_lock_t *lock)
{
    return spin_trylock(lock);
}


#endif
