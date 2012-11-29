/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_QUEUE_H
#define _PHLOX_QUEUE_H

#include <phlox/types.h>

/* queue element */
typedef struct queue_element {
    struct queue_element *next;  /* next element */
} queue_elem_t;

/* queue */
typedef struct {
    queue_elem_t *head;  /* head of queue  */
    queue_elem_t *tail;  /* tail of queue  */
    uint32 count;        /* elements count */
} queue_t;


/* init queue */
uint32 queue_init(queue_t *q);

/* init queue element */
uint32 queue_elem_init(queue_elem_t *e);

/* remove element from queue */
uint32 queue_remove(queue_t *q, queue_elem_t *e);

/* enqueue element */
uint32 queue_enqueue(queue_t *q, queue_elem_t *e);

/* dequeue element */
queue_elem_t *queue_dequeue(queue_t *q);

/* peek queue */
queue_elem_t *queue_peek(queue_t *q);

/* returns true if queue is empty */
bool queue_isempty(queue_t *q);

#endif
