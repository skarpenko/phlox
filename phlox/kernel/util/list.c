/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <phlox/list.h>


uint xlist_init(xlist_t *list)
{
    /* set list to initial state */
    list->first = list->last = NULL;
    list->count = 0;
    return 1;
}

uint xlist_elem_init(list_elem_t *e)
{
    /* set list item to initial state */
    e->prev = e->next = NULL;
    return 1;
}

uint xlist_isempty(xlist_t *list)
{
    if(!list->count)
      return 1;
    else
      return 0;
}

uint xlist_remove(xlist_t *list, list_elem_t *e)
{
    list_elem_t *temp;

    /* search list for given item */
    temp = list->first;
    while(temp) {
       if(temp == e) { /* if item found */
           /* ensure that item is not first */
           if(e->prev)
             e->prev->next = e->next;
           else
             list->first = e->next;

           /* ensure that item is not last */
           if(e->next)
             e->next->prev = e->prev;
           else
             list->last = e->prev;

          list->count--; /* decrement items count */
          e->next = e->prev = NULL; /* set item to initial state */
          return 1; /* return true */
       }
       temp = temp->next; /* get next item */
    }

    /* return false if item was not found */
    return 0;
}

uint xlist_remove_unsafe(xlist_t *list, list_elem_t *e)
{
/*
 * That is unsafe remove. It is not checked that
 * "e" is really in "list".
 */

    /* ensure that item is not first */
    if(e->prev)
      e->prev->next = e->next;
    else
      list->first = e->next;

    /* ensure that item is not last */
    if(e->next)
      e->next->prev = e->prev;
    else
      list->last = e->prev;

    list->count--; /* decrement items count */
    e->next = e->prev = NULL; /* set item to initial state */
    return 1; /* return true */
}

uint xlist_add_first(xlist_t *list, list_elem_t *e)
{
    if(!list->first) {
        /* if list is empty */
        e->prev = e->next = NULL;
        list->first = list->last = e;
    } else {
        /* insert item into list as first */
        list->first->prev = e;
        e->next = list->first;
        e->prev = NULL;
        list->first = e;
    }

    list->count++; /* increase items count */
    return 1;
}

uint xlist_add_last(xlist_t *list, list_elem_t *e)
{
    if(!list->last) {
        /* if list is empty */
        e->prev = e->next = NULL;
        list->first = list->last = e;
    } else {
        /* insert item into list as last */
        list->last->next = e;
        e->prev = list->last;
        e->next = NULL;
        list->last = e;
    }

    list->count++; /* increase items count */
    return 1;
}

list_elem_t *xlist_extract_first(xlist_t *list)
{
    list_elem_t *e;

    e = list->first;
    if(e) {
       /* extract first item if list is not empty */
       list->first = e->next;
       /* ensure that extracted item was not the last in list */
       if(list->first)
         list->first->prev = NULL;
       else
         list->last = NULL;
       list->count--; /* decrease items count */
       e->next = e->prev = NULL; /* set item to initial state */
    }

    return e;
}

list_elem_t *xlist_extract_last(xlist_t *list)
{
    list_elem_t *e;

    e = list->last;
    if(e) {
       /* extract last item if list is not empty */
       list->last = e->prev;
       /* ensure that extracted item was not the last in list */
       if(list->last)
         list->last->next = NULL;
       else
         list->first = NULL;
       list->count--; /* decrease items count */
       e->next = e->prev = NULL; /* set item to initial state */
    }

    return e;
}

uint xlist_insert_after(xlist_t *list, list_elem_t *item, list_elem_t *e)
{
    list_elem_t *temp;

    /* locate given item */
    temp = list->first;
    while(temp) {
       if(temp == item) {
         /* if found, insert after it */
         e->prev = item;
         e->next = item->next;
         if(!e->next)
           list->last = e;
         else
           e->next->prev = e;
         item->next = e;
         list->count++; /* increase items count */
         return 1; /* return true */
       }
       temp = temp->next; /* get next item */
    }

    return 0; /* return false */
}

uint xlist_insert_after_unsafe(xlist_t *list, list_elem_t *item, list_elem_t *e)
{
/*
 * Unsafe insert. It is not checked that "item" is really in "list".
 */

    e->prev = item;
    e->next = item->next;
    if(!e->next)
      list->last = e;
    else
      e->next->prev = e;
    item->next = e;
    list->count++; /* increase items count */
    return 1; /* return true */
}

uint xlist_insert_before(xlist_t *list, list_elem_t *item, list_elem_t *e)
{
    list_elem_t *temp;

    /* locate given item */
    temp = list->first;
    while(temp) {
       if(temp == item) {
       /* if found, insert before it */
         e->prev = item->prev;
         e->next = item;
         if(!e->prev)
           list->first = e;
         else
           e->prev->next = e;
         item->prev = e;
         list->count++; /* increase items count */
         return 1; /* return true */
       }
       temp = temp->next; /* get next item */
    }

    return 0; /* return false */
}

uint xlist_insert_before_unsafe(xlist_t *list, list_elem_t *item, list_elem_t *e)
{
/*
 * Unsafe insert. It is not checked that "item" is really in "list".
 */

    e->prev = item->prev;
    e->next = item;
    if(!e->prev)
      list->first = e;
    else
      e->prev->next = e;
    item->prev = e;
    list->count++; /* increase items count */
    return 1; /* return true */
}

list_elem_t *xlist_peek_first(xlist_t *list)
{
    return list->first;
}

list_elem_t *xlist_peek_last(xlist_t *list)
{
    return list->last;
}

uint xlist_dump_elements(xlist_t *list, list_elem_t **elements)
{
    list_elem_t *temp;
    uint i;

    /* dump all items into given array */
    temp = list->first;
    i = 0;
    while(temp) {
       elements[i++] = temp;
       temp = temp->next;
    }

    return 1;
}
