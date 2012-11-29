/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/processor.h>
#include <phlox/spinlock.h>

void spin_init(spinlock_t *s)
{
    atomic_set(s, SPIN_UNLOCKED);
}

void spin_lock(spinlock_t *s)
{
    /* spin until lock is not released */
    while(atomic_set_ret(s, SPIN_LOCKED)) cpu_relax();
}

void spin_safelock(spinlock_t *s)
{
    /* enable interrupt requests */
    local_irqs_enable();
    /* and spin until lock is not released */
    while(atomic_set_ret(s, SPIN_LOCKED)) cpu_relax();
}

int spin_trylock(spinlock_t *s)
{
    /* try to acquire lock */
    return !atomic_set_ret(s, SPIN_LOCKED);
}

unsigned long spin_lock_irqsave(spinlock_t *s)
{
    unsigned long irqs_state;

    /* save irqs state and disable interrupts */
    local_irqs_save_and_disable(irqs_state);
    /* and then spin until lock is not released */
    while(atomic_set_ret(s, SPIN_LOCKED)) cpu_relax();
    /* return irqs state */
    return irqs_state;
}

void spin_unlock(spinlock_t *s)
{
    atomic_set(s, SPIN_UNLOCKED);
}

void spin_unlock_irqrstor(spinlock_t *s, unsigned long irqs_state)
{
    /* unlock */
    atomic_set(s, SPIN_UNLOCKED);
    /* restore irqs state */
    local_irqs_restore(irqs_state);
}

void spin_wait(spinlock_t *s)
{
    /* spin until lock acquired */
    while(atomic_get(s)) cpu_relax();
}

int spin_locked(spinlock_t *s)
{
    return atomic_get(s);
}
