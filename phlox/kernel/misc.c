/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <phlox/kernel.h>
#include <phlox/heap.h>

/* duplicates string using kernel's heap */
char *kstrdup(const char *text)
{
    char *buf = (char *)kmalloc(strlen(text) + 1);

    if(buf != NULL)
      strcpy(buf,text);

    return buf;
}
