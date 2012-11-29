/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Copyright 2001, Travis Geiselbrecht. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <sys/types.h>

char *strcat(char *dest, char const *src)
{
    char *tmp = dest;

    while(*dest)
        dest++;
    while((*dest++ = *src++) != '\0') ;

    return tmp;
}
