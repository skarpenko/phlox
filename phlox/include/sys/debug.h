/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _SYS_DEBUG_H_
#define _SYS_DEBUG_H_

#ifdef (__KERNEL__ && __DEBUG__)
extern int panic(const char *fmt, ...);
#define ASSERT(ex) \
          if(!(ex)) panic("[ASSERT] File %s, Line %d\n", \
                      __FILE__, __LINE__)
#define ASSERT_MSG(ex,msg) \
          if(!(ex)) panic("[ASSERT] File %s, Line %d: %s\n", \
                      __FILE__, __LINE__, msg)
#else
#define ASSERT(ex)
#define ASSERT_MSG(ex,msg)
#endif

#endif
