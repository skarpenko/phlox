/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Copyright 2001, Travis Geiselbrecht. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <ctype.h>

void *memscan(void *addr, int c, size_t size)
{
    unsigned char *p = (unsigned char *)addr;

    while(size) {
        if(*p == c)
            return (void *)p;
        p++;
        size--;
    }
    return (void *)p;
}
