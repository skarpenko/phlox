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


/*
 * Create semaphore
 * Params:
 *   name       - optional semaphore name;
 *   max_count  - max. value of the semaphore;
 *   init_count - initial value;
 *   owner      - owner process id.
*/
sem_id sem_create_ex(const char *name, uint max_count, uint init_count, proc_id owner);
sem_id sem_create(const char *name, uint max_count, uint init_count);

/*
 * Delete semaphore
*/
status_t sem_delete(sem_id id);

/*
 * Count semaphore down
 * Params:
 *   id           - semaphore id;
 *   count        - value to count down;
 *   timeout_msec - wait timeout in msec;
 *   flags        - flags (see below).
*/
status_t sem_down_ex(sem_id id, uint count, uint timeout_msec, flags_t flags);
status_t sem_down(sem_id id, uint count);

/* Flags to use in sem_down_ex() routine */
enum {
    SEMF_NOFLAGS = 0x0,  /* No flags */
    SEMF_TIMEOUT = 0x1,  /* Use timeout param */
    SEMF_TRY     = 0x2   /* Try to count down */
};

/*
 * Count semaphore up
 * Params:
 *   id    - semaphore id;
 *   count - value to count up.
*/
status_t sem_up(sem_id id, uint count);

/*
 * Find semaphore by name
*/
sem_id sem_get_by_name(const char *name);

/*
 * Change semaphore owning process
 * Params:
 *   id        - semaphore id;
 *   new_owner - new owning process id.
*/
status_t sem_change_owner(sem_id id, proc_id new_owner);


#endif
