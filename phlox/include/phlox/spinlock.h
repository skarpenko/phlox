/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
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
 * Acquire lock with disabled interrupt requests.
 * Returns previous interrupt requests state.
*/
unsigned long spin_lock_irqsave(spinlock_t *s);

/*
 * Release lock
*/
void spin_unlock(spinlock_t *s);

/*
 * Release lock and restore previously saved interrupt
 * requests state.
*/
void spin_unlock_irqrstor(spinlock_t *s, unsigned long irqs_state);

/*
 * Wait for lock release
*/
void spin_wait(spinlock_t *s);

/*
 * Returns true if locked
*/
int spin_locked(spinlock_t *s);

/*
 * SMP-specific stuff. Must only be used to control SMP effects.
 */
#if SYSCFG_SMP_SUPPORT
  #define smp_spin_lock(s)    spin_lock(s)
  #define smp_spin_unlock(s)  spin_unlock(s)
  #define smp_spin_wait(s)    spin_wait(s)
#else
  #define smp_spin_lock(s)
  #define smp_spin_unlock(s)
  #define smp_spin_wait(s)
#endif

#endif
