/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/queue.h>


uint queue_init(queue_t *q)
{
    /* set initial state of queue */
    q->head = q->tail = NULL;
    q->count = 0;
    return 1;
}

uint queue_elem_init(queue_elem_t *e)
{
    /* set initial state of queue element */
    e->next = NULL;
    return 1;
}

uint queue_remove(queue_t *q, queue_elem_t *e)
{
    queue_elem_t *temp, *last = NULL;

    /* locate element */
    temp = q->head;
    while(temp) {
      if(temp == e) {
        /* remove element */
        if(last) {
            last->next = temp->next;
        } else {
            q->head = temp->next;
        }
        if(q->tail == temp)
            q->tail = last;
        q->count--; /* decrease elements count */
        e->next = NULL; /* set element to initial state */
        return 1;   /* return true */
      }
      last = temp;       /* store previous */
      temp = temp->next; /* get next */
    }

    /* return false if element was not found */
    return 0;
}

uint queue_enqueue(queue_t *q, queue_elem_t *e)
{
    if(q->tail == NULL) {
        /* if queue empty */
        q->tail = e;
        q->head = e;
    } else {
        /* and if not */
        q->tail->next = e;
        q->tail = e;
    }
    e->next = NULL; /* it is now last element in queue */
    q->count++; /* increase elements count */
    return 1;
}

queue_elem_t *queue_dequeue(queue_t *q)
{
    queue_elem_t *e;

    /* extract element from queue */
    e = q->head;
    if(q->head != NULL)
        q->head = q->head->next;
    if(q->tail == e)
        q->tail = NULL;

    /* decrease elements count if
     * element was extracted
     */
    if(e != NULL)
        q->count--;

    return e;
}

queue_elem_t *queue_peek(queue_t *q)
{
    return q->head;
}

bool queue_isempty(queue_t *q)
{
   if(!q->count)
     return true;
   else
     return false;
}
