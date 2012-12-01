/*
* Copyright 2007-2012, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/errors.h>
#include <phlox/sem.h>
#include <phlox/thread.h>
#include <phlox/mutex.h>



/* init mutex data */
void mutex_init(mutex_t *mtx)
{
    mtx->sem = INVALID_SEMID;
    mtx->holder = INVALID_THREADID;
}

/* create mutex */
status_t mutex_create(mutex_t *mtx, const char *name)
{
    /* init holder field and create semaphore */
    mtx->holder = INVALID_THREADID;
    mtx->sem = sem_create(name, 1, 1);

    /* return status */
    if(mtx->sem == INVALID_SEMID)
        return ERR_SEM_GENERAL;
    else
        return NO_ERROR;
}

/* destroy mutex */
status_t mutex_destroy(mutex_t *mtx)
{
    status_t retc = sem_delete(mtx->sem);

    mtx->sem = INVALID_SEMID;
    mtx->holder = INVALID_THREADID;

    return retc;
}

/* acquire mutex */
status_t mutex_lock(mutex_t *mtx)
{
    status_t retc = ERR_SEM_GENERAL;

    if(mtx->sem != INVALID_SEMID)
        retc = sem_down(mtx->sem, 1);

    if(retc == NO_ERROR)
        mtx->holder = thread_get_current_thread_id();

    return retc;
}

/* release mutex */
status_t mutex_unlock(mutex_t *mtx)
{
    if(mtx->sem == INVALID_SEMID)
        return ERR_SEM_GENERAL;

    if(mtx->holder != thread_get_current_thread_id())
        return ERR_SEM_GENERAL;

    mtx->holder = INVALID_THREADID;

    sem_up(mtx->sem, 1);

    return NO_ERROR;
}
