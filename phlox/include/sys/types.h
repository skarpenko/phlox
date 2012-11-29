/*
* Copyright 2007, Stepan V.Karpenko. All rights reserved.
* Copyright 2004, Travis Geiselbrecht. All rights reserved.
* Copyright 2002, Manuel J. Petit. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/

#ifndef __SYS_TYPES_H
#define __SYS_TYPES_H

#include <phlox/types.h>
#include <endian.h>

typedef proc_id    pid_t;
typedef thread_id  tid_t;
typedef vnode_id   ino_t;
typedef vnode_id   dev_t;
typedef uint32     mode_t;
typedef uint32     nlink_t;
typedef uint32     uid_t;
typedef uint32     gid_t;
typedef uint64     blkcnt_t;
typedef size_t     blksize_t;

#endif
