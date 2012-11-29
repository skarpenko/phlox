/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _BOOTDECOMP_INFLATE_H
#define _BOOTDECOMP_INFLATE_H

unsigned long gunzip(const unsigned char *in, unsigned char *out,
                     unsigned char *inf_buf);

#endif
