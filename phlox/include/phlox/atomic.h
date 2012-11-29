/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ATOMIC_H
#define _PHLOX_ATOMIC_H

/***
 *** Atomic operations that C can't guarantee us.  Useful for
 *** resource counting etc..
 ***/

/* Atomic type */
typedef volatile int atomic_t;


/*
 * Atomically sets the value of *a to set_to
 */
void atomic_set(atomic_t *a, int set_to);

/*
 * Atomically sets the value of *a to set_to
 * and returns previous value of *a
 */
int  atomic_set_ret(atomic_t *a, int set_to);

/*
 * Atomically compares the value of *a and test_val
 * and sets the value of *a to set_to if *a = test_val
 *
 * Returns true if the value of *a was changed
 */
int  atomic_test_and_set(atomic_t *a, int set_to, int test_val);

/*
 * Atomically reads the value of *a
 */
int  atomic_get(atomic_t *a);

/*
 * Atomically adds the value of v to *a
 */
void atomic_add(atomic_t *a, int v);

/*
 * Atomically subtracts the value of v from *a
 */
void atomic_sub(atomic_t *a, int v);

/*
 * Atomically increments the value of *a
 */
void atomic_inc(atomic_t *a);

/*
 * Atomically decrements the value of *a
 */
void atomic_dec(atomic_t *a);

/*
 * Atomically changes sign of *a
 */
void atomic_neg(atomic_t *a);

/*
 * Atomically adds the value of v to *a and
 * returns previous value of *a
 */
int  atomic_add_ret(atomic_t *a, int v);

/*
 * Atomically subtracts the value of v from *a and
 * returns previous value of *a
 */
int  atomic_sub_ret(atomic_t *a, int v);

/*
 * Atomically increments the value of *a
 * and returns its previous value
 */
int  atomic_inc_ret(atomic_t *a);

/*
 * Atomically decrements the value of *a
 * and returns its previous value
 */
int  atomic_dec_ret(atomic_t *a);

/*
 * Atomically changes sign of *a and
 * returns its previous value
 */
int  atomic_neg_ret(atomic_t *a);

/*
 * Atomic logical NOT operation
 */
void atomic_not(atomic_t *a);

/*
 * Atomic logical AND operation
 */
void atomic_and(atomic_t *a, int v);

/*
 * Atomic logical OR operation
 */
void atomic_or(atomic_t *a, int v);

/*
 * Atomic logical XOR operation
 */
void atomic_xor(atomic_t *a, int v);

/*
 * Atomically NOTs the value of *a
 * and returns its previous value
 */
int  atomic_not_ret(atomic_t *a);

/*
 * Atomically ANDs the values of v and *a
 * and returns previous value of *a
 */
int  atomic_and_ret(atomic_t *a, int v);

/*
 * Atomically ORs the values of v and *a
 * and returns previous value of *a
 */
int  atomic_or_ret(atomic_t *a, int v);

/*
 * Atomically XORs the values of v and *a
 * and returns previous value of *a
 */
int  atomic_xor_ret(atomic_t *a, int v);

/*
 * Atomically adds the value of v to *a
 * and returns true if result is zero
 */
int  atomic_add_and_testz(atomic_t *a, int v);

/*
 * Atomically subtracts the value of v from *a
 * and returns true if result is zero
 */
int  atomic_sub_and_testz(atomic_t *a, int v);

/*
 * Atomically increments the value of *a
 * and returns true if result is zero
 */
int  atomic_inc_and_testz(atomic_t *a);

/*
 * Atomically decrements the value of *a
 * and returns true if result is zero
 */
int  atomic_dec_and_testz(atomic_t *a);

/*
 * Atomically adds the value of v to *a
 * and returns true if result is negative
 */
int  atomic_add_and_tests(atomic_t *a, int v);

/*
 * Atomically subtracts the value of v from *a
 * and returns true if result is negative
 */
int  atomic_sub_and_tests(atomic_t *a, int v);

/*
 * Atomically increments the value of *a
 * and returns true if result is negative
 */
int  atomic_inc_and_tests(atomic_t *a);

/*
 * Atomically decrements the value of *a
 * and returns true if result is negative
 */
int  atomic_dec_and_tests(atomic_t *a);

#endif
