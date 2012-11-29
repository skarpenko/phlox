/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Copyright 2001, Travis Geiselbrecht. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <sys/types.h>

int strcmp(char const *cs, char const *ct)
{
    signed char __res;

    while(1) {
        if((__res = *cs - *ct++) != 0 || !*cs++)
            break;
    }

    return __res;
}
