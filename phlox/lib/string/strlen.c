/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <sys/types.h>

size_t strlen(char const *s)
{
    size_t i;

    i= 0;
    while(s[i]) i++;

    return i;
}
