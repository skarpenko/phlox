/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Copyright 2001, Manuel J. Petit. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <sys/types.h>

char *strrchr(char const *s, int c)
{
    char const *last= c?0:s;


    while(*s) {
        if(*s== c) {
            last= s;
        }

        s+= 1;
    }

    return (char *)last;
}
