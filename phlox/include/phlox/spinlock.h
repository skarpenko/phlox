/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_SPINLOCK_H_
#define _PHLOX_SPINLOCK_H_

#include <phlox/atomic.h>

/* Spinlock states */
#define SPIN_LOCKED    1
#define SPIN_UNLOCKED  0

/* Spinlock typedef */
typedef atomic_t spinlock_t;

/*
 * Set spinlock variable to initial state
*/
void spin_init(spinlock_t *s);

/*
 * Acquire lock
*/
void spin_lock(spinlock_t *s);

/*
 * Enable interrupt requests and acquire lock
*/
void spin_safelock(spinlock_t *s);

/*
 * Try to aquire lock.
 * Returns true if lock was aquired.
*/
int spin_trylock(spinlock_t *s);

/*
 * Release lock
*/
void spin_unlock(spinlock_t *s);

/*
 * Wait for lock release
*/
void spin_wait(spinlock_t *s);

/*
 * Returns true if locked
*/
int spin_locked(spinlock_t *s);

#endif
