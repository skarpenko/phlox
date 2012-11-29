/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_LISTS_H
#define _PHLOX_LISTS_H

#include <phlox/types.h>
#include <phlox/kernel.h>


/* double-linked list element */
typedef struct list_element {
    struct list_element *prev;  /* previous element in the list */
    struct list_element *next;  /* next element in the list     */
} list_elem_t;

/* multi-purpose list */
typedef struct {
    list_elem_t *first;  /* first element in the list */
    list_elem_t *last;   /* last element in the list  */
    uint count;
} xlist_t;


/*** Routines for multi-purpose list manipulations ***/

/* init list */
uint xlist_init(xlist_t *list);

/* init list element */
uint xlist_elem_init(list_elem_t *e);

/* returns true if list is empty */
uint xlist_isempty(xlist_t *list);

/* remove element from list */
uint xlist_remove(xlist_t *list, list_elem_t *e);

/* remove element from list in unsafe manner
 * Note: it is not checked that element is really
 * lies in given list.
 */
uint xlist_remove_unsafe(xlist_t *list, list_elem_t *e);

/* add element to list as first */
uint xlist_add_first(xlist_t *list, list_elem_t *e);

/* add element to list as last */
uint xlist_add_last(xlist_t *list, list_elem_t *e);

/* extract first element from list */
list_elem_t *xlist_extract_first(xlist_t *list);

/* extract last element from list */
list_elem_t *xlist_extract_last(xlist_t *list);

/* insert element to list after given item
 * Note: firstly routine searches list for item to ensure
 * list integrity.
 */
uint xlist_insert_after(xlist_t *list, list_elem_t *item, list_elem_t *e);

/* insert elemet to list after given item in unsafe manner
 * Note: no additional list integrity checking performed.
 */
uint xlist_insert_after_unsafe(xlist_t *list, list_elem_t *item, list_elem_t *e);

/* insert element to list before given item
 * Note: firstly routine searches list for item to ensure
 * list integrity.
 */
uint xlist_insert_before(xlist_t *list, list_elem_t *item, list_elem_t *e);

/* insert elemet to list before given item in unsafe manner
 * Note: no additional list integrity checking performed.
 */
uint xlist_insert_before_unsafe(xlist_t *list, list_elem_t *item, list_elem_t *e);

/* peek first element of the list */
list_elem_t *xlist_peek_first(xlist_t *list);

/* peek last element of the list */
list_elem_t *xlist_peek_last(xlist_t *list);

/* dump all elements into array */
uint xlist_dump_elements(xlist_t *list, list_elem_t **elements);

/* default add operation */
#define xlist_add(list, e)      xlist_add_last(list, e)

/* queue operations */
#define xlist_enqueue(list, e)  xlist_add_last(list, e)
#define xlist_dequeue(list)     xlist_extract_first(list)


/***
 * Routines for circular double-linked list manipulations
 * (Ported from NewOS)
 ***/

/* init list */
static inline void clist_init(list_elem_t *list)
{
    list->prev = list->next = list;
}

/* clear list item */
static inline void clist_elem_clear(list_elem_t *e)
{
    e->prev = e->next = NULL;
}

/* add item into head of the list */
static inline void clist_add_head(list_elem_t *list, list_elem_t *e)
{
    e->next = list->next;
    e->prev = list;
    list->next->prev = e;
    list->next = e;
}

/* add item into tail of the list */
static inline void clist_add_tail(list_elem_t *list, list_elem_t *e)
{
    e->prev = list->prev;
    e->next = list;
    list->prev->next = e;
    list->prev = e;
}

/* remove item from list */
static inline void clist_remove(list_elem_t *e)
{
    e->next->prev = e->prev;
    e->prev->next = e->next;
    /* init item to initial state */
    e->prev = e->next = NULL;
}

/* extract head item */
static inline list_elem_t *clist_extract_head(list_elem_t *list)
{
    if(list->next != list) {
        list_elem_t *item = list->next;
        clist_remove(item);
        return item;
    } else {
        return NULL;
    }
}

/* extract head item. typed version. */
#define clist_extract_head_type(list, type, element) ({\
    list_elem_t *__nod = clist_extract_head(list);\
    type *__t;\
    if(__nod)\
        __t = containerof(__nod, type, element);\
    else\
        __t = (type *)0;\
    __t;\
})

/* extract tail item */
static inline list_elem_t *clist_extract_tail(list_elem_t *list)
{
    if(list->prev != list) {
        list_elem_t *item = list->prev;
        clist_remove(item);
        return item;
    } else {
        return NULL;
    }
}

/* extract tail item. typed version. */
#define clist_extract_tail_type(list, type, element) ({\
    list_elem_t *__nod = clist_extract_tail(list);\
    type *__t;\
    if(__nod)\
        __t = containerof(__nod, type, element);\
    else\
        __t = (type *)0;\
    __t;\
})

/* peek head item */
static inline list_elem_t* clist_peek_head(list_elem_t *list)
{
    if(list->next != list) {
        return list->next;
    } else {
        return NULL;
    }   
}

/* peek head item. typed version. */
#define clist_peek_head_type(list, type, element) ({\
    list_elem_t *__nod = clist_peek_head(list);\
    type *__t;\
    if(__nod)\
        __t = containerof(__nod, type, element);\
    else\
        __t = (type *)0;\
    __t;\
})

/* peek tail item */
static inline list_elem_t* clist_peek_tail(list_elem_t *list)
{
    if(list->prev != list) {
        return list->prev;
    } else {
        return NULL;
    }   
}

/* peek tail item. typed version. */
#define clist_peek_tail_type(list, type, element) ({\
    list_elem_t *__nod = clist_peek_tail(list);\
    type *__t;\
    if(__nod)\
        __t = containerof(__nod, type, element);\
    else\
        __t = (type *)0;\
    __t;\
})

/* get next item */
static inline list_elem_t* clist_next(list_elem_t *list, list_elem_t *item)
{
    if(item->next != list)
        return item->next;
    else
        return NULL;
}

/* get next item. typed version. */
#define clist_next_type(list, item, type, element) ({\
    list_elem_t *__nod = clist_next(list, item);\
    type *__t;\
    if(__nod)\
        __t = containerof(__nod, type, element);\
    else\
        __t = (type *)0;\
    __t;\
})

/* get next item with wrapping */
static inline list_elem_t* clist_next_wrap(list_elem_t *list, list_elem_t *item)
{
    if(item->next != list)
        return item->next;
    else if(item->next->next != list)
        return item->next->next;
    else
        return NULL;
}

/* get next item with wrapping. typed version. */
#define clist_next_wrap_type(list, item, type, element) ({\
    list_elem_t *__nod = clist_next_wrap(list, item);\
    type *__t;\
    if(__nod)\
        __t = containerof(__nod, type, element);\
    else\
        __t = (type *)0;\
    __t;\
})

/* iterates over the list, node should be struct list_elem_t* */
#define clist_for_every(list, node) \
    for(node = (list)->next; node != (list); node = node->next)

/* iterates over the list in a safe way for deletion of current node
 * node and temp_node should be struct list_elem_t*
*/
#define clist_for_every_safe(list, node, temp_node) \
    for(node = (list)->next, temp_node = (node)->next;\
    node != (list);\
    node = temp_node, temp_node = (node)->next)

/* iterates over the list, entry should be the container structure type* */
#define clist_for_every_entry(list, entry, type, member) \
    for((entry) = containerof((list)->next, type, member);\
        &(entry)->member != (list);\
        (entry) = containerof((entry)->member.next, type, member))

/* iterates over the list in a safe way for deletion of current node
 * entry and temp_entry should be the container structure type*
*/
#define clist_for_every_entry_safe(list, entry, temp_entry, type, member) \
    for(entry = containerof((list)->next, type, member),\
        temp_entry = containerof((entry)->member.next, type, member);\
        &(entry)->member != (list);\
        entry = temp_entry, temp_entry = containerof((temp_entry)->member.next, type, member))

/* returns true if list is empty */
static inline bool clist_isempty(list_elem_t *list)
{
    return (list->next == list) ? true : false;
}

/* returns count of items in list */
static inline uint clist_count(list_elem_t *list)
{
    uint res = 0;
    list_elem_t *temp = list;

    do {
       res++;
       temp = temp->next;
    } while(temp != list);

    return res;
}

/* dumps all list elements into array */
static inline void clist_dump_elements(list_elem_t *list, list_elem_t **elements)
{
    uint i = 0;
    list_elem_t *temp = list;

    do {
       elements[i++] = temp;
       temp = temp->next;
    } while(temp != list);
}

#endif
