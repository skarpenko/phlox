/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_THREAD_PRIVATE_H_
#define _PHLOX_THREAD_PRIVATE_H_

/*******************************************************************
 * THREADING INTERNALS. DO NOT USE!
 *******************************************************************/

/* Thread kernel-side stack size */
#define THREAD_KSTACK_SIZE  (KERNEL_STACK_SIZE*PAGE_SIZE)
/* Thread kernel-side stack warning value */
#define THREAD_KSTACK_WARN  (THREAD_KSTACK_SIZE/8)
/*
 * Thread kernel-side stack layout
 *
 * |-------| <- Top = Base + THREAD_KSTACK_SIZE - 1
 * |-------|
 * |-------|
 * |-------|
 * |-------|
 * |-------|
 * |-------| <- Danger stack usage barrier (Base + THREAD_KSTACK_WARN)
 * |--Ptr--| <- Base
 *
 * Ptr - pointer to thread_t structure.
*/


/* Minimum and maximum scheduling quanta can be set for thread (in milliseconds) */
#define THREAD_DEFAULT_QUANTA     (60)
#define THREAD_MINIMUM_QUANTA     (20)
#define THREAD_QUANTA_GRANULARITY (10)
/* quanta params...*/
/// /* def */, /* min */, /* granularity */

/* Priorities constants */
#define THREAD_NUM_PRIORITY_LEVELS  (128)
#define THREAD_LOWEST_PIORITY       (0)  /* 0 is for per CPU idle threads */
#define THREAD_HIGHEST_PRIORITY     (THREAD_NUM_PRIORITY_LEVELS-1)

/* Additional priority levels added to thread depending on process role */
///
#define PROCESS_ROLE_PRIORITY_SHIFT_KERNEL  (8)
#define PROCESS_ROLE_PRIORITY_SHIFT_SERVICE  (4)
#define PROCESS_ROLE_PRIORITY_SHIFT_USER     (0)


#endif
