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
        return ERR_MTX_SEM_FAILURE;
    else
        return NO_ERROR;
}

/* destroy mutex */
status_t mutex_destroy(mutex_t *mtx)
{
    status_t retc = sem_delete(mtx->sem);

    mtx->sem = INVALID_SEMID;
    mtx->holder = INVALID_THREADID;

    return (retc == NO_ERROR) ? NO_ERROR : ERR_MTX_SEM_FAILURE;
}

/* acquire mutex */
status_t mutex_lock(mutex_t *mtx)
{
    status_t retc = ERR_MTX_INVALID_MUTEX;

    if(mtx->sem != INVALID_SEMID)
        retc = (sem_down(mtx->sem, 1) == NO_ERROR) ? NO_ERROR : ERR_MTX_SEM_FAILURE;

    if(retc == NO_ERROR)
        mtx->holder = thread_get_current_thread_id();

    return retc;
}

/* try acquire mutex */
status_t mutex_trylock(mutex_t *mtx)
{
    status_t retc = ERR_MTX_INVALID_MUTEX;

    if(mtx->sem != INVALID_SEMID) {
        switch( sem_down_ex(mtx->sem, 1, 0, SEMF_TRY) ) {
            case NO_ERROR:
              retc = NO_ERROR;
              break;
            case ERR_SEM_TRY_FAILED:
              retc = ERR_MTX_TRY_FAILED;
              break;
            default:
              retc = ERR_MTX_SEM_FAILURE;
              break;
        }
    }

    if(retc == NO_ERROR)
        mtx->holder = thread_get_current_thread_id();

    return retc;
}

/* release mutex */
status_t mutex_unlock(mutex_t *mtx)
{
    if(mtx->sem == INVALID_SEMID)
        return ERR_MTX_INVALID_MUTEX;

    if(mtx->holder != thread_get_current_thread_id())
        return ERR_MTX_NOT_AN_OWNER;

    mtx->holder = INVALID_THREADID;

    sem_up(mtx->sem, 1);

    return NO_ERROR;
}

/* init recursive mutex data */
void rmutex_init(rmutex_t *mtx)
{
    mtx->sem = INVALID_SEMID;
    mtx->holder = INVALID_THREADID;
    mtx->recurs = 0;
}

/* create recursive mutex */
status_t rmutex_create(rmutex_t *mtx, const char *name)
{
    /* init holder and recursion fields and create semaphore */
    mtx->holder = INVALID_THREADID;
    mtx->recurs = 0;
    mtx->sem = sem_create(name, 1, 1);

    /* return status */
    if(mtx->sem == INVALID_SEMID)
        return ERR_MTX_SEM_FAILURE;
    else
        return NO_ERROR;
}

/* destroy recursive mutex */
status_t rmutex_destroy(rmutex_t *mtx)
{
    status_t retc = sem_delete(mtx->sem);

    mtx->sem = INVALID_SEMID;
    mtx->holder = INVALID_THREADID;
    mtx->recurs = 0;

    return (retc == NO_ERROR) ? NO_ERROR : ERR_MTX_SEM_FAILURE;
}

/* acquire recursive mutex */
status_t rmutex_lock(rmutex_t *mtx)
{
    status_t r = sem_down_ex(mtx->sem, 1, 0, SEMF_TRY);
    thread_id tid = thread_get_current_thread_id();

    /* check for errors */
    if(r != NO_ERROR && r != ERR_SEM_TRY_FAILED)
        return ERR_MTX_SEM_FAILURE;

    if(r == NO_ERROR) {  /* semaphore successfully acquired */
        mtx->holder = tid;
        mtx->recurs = 0;
    } else if(r == ERR_SEM_TRY_FAILED && mtx->holder != tid) { /* not an owner */
        if( sem_down(mtx->sem, 1) ) /* wait on semaphore */
            return ERR_MTX_SEM_FAILURE;
        mtx->holder = tid;
        mtx->recurs = 0;
    }

    /* increment recursion level */
    ++mtx->recurs;

    return NO_ERROR;
}

/* try acquire recursive mutex */
status_t rmutex_trylock(rmutex_t *mtx)
{
    status_t r = sem_down_ex(mtx->sem, 1, 0, SEMF_TRY);
    thread_id tid = thread_get_current_thread_id();

    /* check for errors */
    if(r != NO_ERROR && r != ERR_SEM_TRY_FAILED)
        return ERR_MTX_SEM_FAILURE;

    if(r == NO_ERROR) {  /* semaphore successfully acquired */
        mtx->holder = tid;
        mtx->recurs = 0;
    } else if(r == ERR_SEM_TRY_FAILED && mtx->holder != tid) { /* not an owner */
        return ERR_MTX_TRY_FAILED;
    }

    /* increment recursion level */
    ++mtx->recurs;

    return NO_ERROR;
}

/* release recursive mutex */
status_t rmutex_unlock(rmutex_t *mtx)
{
    if(mtx->sem == INVALID_SEMID)
        return ERR_MTX_INVALID_MUTEX;

    if(mtx->holder != thread_get_current_thread_id())
        return ERR_MTX_NOT_AN_OWNER;

    /* countdown recursion level */
    if(!--mtx->recurs) {
        mtx->holder = INVALID_THREADID;
        sem_up(mtx->sem, 1);
    }

    return NO_ERROR;
}
