/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_SPINLOCK_H_
#define _PHLOX_SPINLOCK_H_

#include <atomic.h>

/* Spinlock states */
#define SPIN_LOCKED    1
#define SPIN_UNLOCKED  0

/* Spinlock typedef */
typedef atomic_t spinlock_t;


#endif
