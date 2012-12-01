/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_THREAD_TYPES_H_
#define _PHLOX_THREAD_TYPES_H_

#include <phlox/ktypes.h>
#include <phlox/list.h>
#include <phlox/avl_tree.h>
#include <phlox/vm_types.h>
#include <phlox/processor.h>
#include <phlox/spinlock.h>
#include <phlox/arch/thread_types.h>


/* Thread */
typedef struct thread {
    thread_id        id;                 /* Thread id */
    char             *name;              /* Thread name (can be NULL) */
    struct process   *process;           /* Thread owner process */
    /* Processor */
    processor_t      *cpu;               /* CPU on which thread is executed */
    /* Kernel-side stack data */
    object_id        kstack_id;          /* Kernel stack vm_object id */
    addr_t           kstack_top;         /* Kernel stack top */
    size_t           kstack_base;        /* Kernel stack base */
    /* User-side stack data */
    object_id        ustack_id;          /* User stack vm_object id */
    addr_t           ustack_top;         /* User stack top */
    size_t           ustack_base;        /* User stack base */
    /* Thread state params */
    int              state;              /* Thread state */
    uint             flags;              /* Thread flags */
    int              preemt_count;       /* Thread is not preempted while >0 */
    bool             in_kernel;          /* =true if in kernel */
    /* Thread timing */
    bigtime_t        kernel_time;        /* Kernel-side execution time */
    bigtime_t        user_time;          /* User-side execution time */
    /* Entry */
    addr_t           entry;              /* Entry point address */
    void             *data;              /* Optional data passed to thread */
    /* List nodes */
    list_elem_t      threads_list_node;  /* Threads list node */
    avl_tree_node_t  threads_tree_node;  /* Threads tree node */
    list_elem_t      proc_list_node;     /* Process threads list node */
    list_elem_t      sched_list_node;    /* Scheduler's list node */
    /* Hardware-dependend data */
    arch_thread_t    arch;               /* Architecture-dependend data */
/* TODO: Priorities, Semaphores */
} thread_t;

/* Thread states */
enum {
  THREAD_STATE_READY = 0,  /* Ready for execution */
  THREAD_STATE_BIRTH,      /* Thread is in creation stage */
  THREAD_STATE_DEATH,      /* Thread destroying */
  THREAD_STATE_RUNNING,    /* Thread running */
  THREAD_STATE_WAITING,    /* Thread is waiting for event */
  THREAD_STATE_SUSPENDED,  /* Thread is suspended */
  THREAD_STATE_DEAD        /* Thread is dead */
};

/* Process */
typedef struct process {
    proc_id             id;               /* Process id */
    pgrp_id             gid;              /* Process group id */
    sess_id             sid;              /* Session id */
    char                *name;            /* Process name */
    char                *args;            /* Arguments passed to process */
    struct process      *parent;          /* Parent process */
    /* Process state params */
    int                 state;            /* Process state */
    uint                flags;            /* Process flags */
    vuint               ref_count;        /* Reference count */
    /* Process memory */
    aspace_id           aid;              /* Address space id */
    vm_address_space_t  *aspace;          /* Address space */
    /* Internal data structures */
    spinlock_t          lock;             /* Lock for lists access */
    struct thread       *main;            /* Main thread */
    xlist_t             threads;          /* Threads associated with process */
    xlist_t             children;         /* List of child processes */
    list_elem_t         sibling_node;     /* Node in parent process */
    /* Node of global processes list and tree */
    list_elem_t         procs_list_node;  /* Processes list node */
    avl_tree_node_t     procs_tree_node;  /* Processes tree node */
    /* Hardware-dependend data */
    arch_process_t       arch;             /* Architecture-dependend data */
} process_t;

/* Process states */
enum {
  PROCESS_STATE_NORMAL = 0,  /* Normal state */
  PROCESS_STATE_BIRTH,       /* Process is in creation stage */
  PROCESS_STATE_DEATH        /* Process is in destroying stage */
};

#endif
