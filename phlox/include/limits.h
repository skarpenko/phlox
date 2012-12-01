/*
* Copyright 2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _LIMITS_H_
#define _LIMITS_H_


/* Minimum and maximum values a "signed char" can hold. */
#define SCHAR_MIN  (-128)
#define SCHAR_MAX  127

/* Maximum value an "unsigned char" can hold. (Minimum is 0) */
#define UCHAR_MAX  255

/* Minimum and maximum values a "char" can hold. */
#define CHAR_MIN  SCHAR_MIN
#define CHAR_MAX  SCHAR_MAX

/* Minimum and maximum values a "signed short int" can hold. */
#define SHRT_MIN  (-32768)
#define SHRT_MAX  32767

/* Maximum value an "unsigned short int" can hold. (Minimum is 0) */
#define USHRT_MAX  65535

/* Minimum and maximum values a "signed int" can hold. */
#define INT_MIN  (-INT_MAX - 1)
#define INT_MAX  ((int)(~0U>>1))

/* Maximum value an "unsigned int" can hold. (Minimum is 0) */
#define UINT_MAX  (~0U)

/* Minimum and maximum values a "signed long int" can hold. */
#define LONG_MIN  (-LONG_MAX - 1L)
#define LONG_MAX  ((long)(~0UL>>1))

/* Maximum value an "unsigned long int" can hold. (Minimum is 0) */
#define ULONG_MAX  (~0UL)

/* Minimum and maximum values a "signed long long int" can hold. */
#define LLONG_MIN  (-LLONG_MAX - 1LL)
#define LLONG_MAX  ((long long)(~0ULL>>1))

/* Maximum value an "unsigned long long int" can hold. (Minimum is 0) */
#define ULLONG_MAX  (~0ULL)


#endif
