/*
* Copyright 2007-2011, Stepan V.Karpenko. All rights reserved.
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


/* Scheduling policy */
typedef union {
    struct {
        uint type : 4;   /* Scheduling policy type */
    } policy;            /* Scheduling policy structure */
    uint raw;            /* Raw value of scheduling policy */
} sched_policy_t;

/* Scheduling policies names */
enum {
  SCHED_POLICY_ORDINARY = 0,   /* Ordinary shceduling policy for most tasks */
  SCHED_POLICY_INTERACTIVE,    /* Sheduling for interactive threads
                                * (actual only when process is interactive)
                                */
  SCHED_POLICY_SERVICE,        /* Scheduling policy for system services */
  SCHED_POLICY_REAL_TIME,      /* Scheduling policy for real-time tasks */
  SCHED_POLICY_KERNEL,         /* Policy for scheduling kernel tasks */
  SCHED_POLICY_HW_HANDLER,     /* Hardware I/O operations handler */
  SCHED_POLICY_INTERRUPT,      /* Policy for low-priority interrupts handling */
  /* Count of scheduling policies */
  SCHED_POLICIES_COUNT
};

/* Thread static priorities */
enum {
  THREAD_PRIORITY_REAL_TIME = 104,
  THREAD_PRIORITY_VERY_HIGH =  88,
  THREAD_PRIORITY_HIGH      =  72,
  THREAD_PRIORITY_NORMAL    =  56,
  THREAD_PRIORITY_LOW       =  40,
  THREAD_PRIORITY_VERY_LOW  =  24,
  THREAD_PRIORITY_IDLE      =   8
};

/* Thread */
typedef struct thread {
    thread_id        id;                 /* Thread id */
    char             *name;              /* Thread name (can be NULL) */
    struct process   *process;           /* Thread owner process */
    /* Self-preservation */
    spinlock_t       lock;               /* Lock for rd/wr dynamic fields */
    /* Processor */
    processor_t      *cpu;               /* CPU on which thread is executed */
    /* Kernel-side stack data */
    object_id        kstack_id;          /* Kernel stack vm_object id */
    addr_t           kstack_top;         /* Kernel stack top */
    addr_t           kstack_base;        /* Kernel stack base */
    /* User-side stack data */
    object_id        ustack_id;          /* User stack vm_object id */
    addr_t           ustack_top;         /* User stack top */
    addr_t           ustack_base;        /* User stack base */
    /* Thread state params */
    int              state;              /* Thread state */
    int              next_state;         /* Thread state after reschedule */
    uint             flags;              /* Thread flags */
    int              preempt_count;      /* Thread is not preempted while >0 */
    bool             in_kernel;          /* =true if in kernel */
    /* Fields used by scheduler */
    int              jiffies;            /* Current jiffies count */
    int              ijiffies;           /* Initial jiffies count */
    sched_policy_t   sched_policy;       /* Scheduling policy */
    int              s_prio;             /* Static priority */
    int              d_prio;             /* Current dynamic priority */
    bigtime_t        sched_stamp;        /* Scheduler's timestamp */
    int              carrots_sticks;     /* Carrots and Sticks Policy value */
    /* Thread timing */
    bigtime_t        kernel_time;        /* Kernel-side execution time */
    bigtime_t        user_time;          /* User-side execution time */
    /* Entry */
    addr_t           entry;              /* Entry point address */
    void             *data;              /* Optional data passed to thread */
    /* Thread termination control */
    xlist_t          term_cbs_list;      /* Termination callbacks list */
    /* List nodes */
    list_elem_t      threads_list_node;  /* Threads list node */
    avl_tree_node_t  threads_tree_node;  /* Threads tree node */
    list_elem_t      proc_list_node;     /* Process threads list node */
    list_elem_t      sched_list_node;    /* List node for execution scheduling */
    /* Hardware-dependend data */
    arch_thread_t    arch;               /* Architecture-dependend data */
/* TODO: Semaphores */
} thread_t;

/* Thread states */
enum {
  THREAD_STATE_READY = 0,  /* Ready for execution */
  THREAD_STATE_BIRTH,      /* Thread is in creation stage */
  THREAD_STATE_DEATH,      /* Thread destroying */
  THREAD_STATE_RUNNING,    /* Thread running */
  THREAD_STATE_WAITING,    /* Thread is waiting for event */
  THREAD_STATE_SLEEPING,   /* Thread is sleeping */
  THREAD_STATE_SUSPENDED,  /* Thread is suspended */
  THREAD_STATE_DEAD        /* Thread is dead */
};

/* Thread flags */
enum {
  THREAD_FLAG_NONE        = 0x0,  /* No flags set */
  THREAD_FLAG_RESCHEDULE  = 0x1   /* Thread will be rescheduled soon */
};
/*
 * NOTE: THREAD_FLAG_RESCHEDULE flag used by scheduler's timer handler
 *       for marking thread as being rescheduled soon.
 */

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
    /* Scheduling data */
    uint                process_role;     /* Process role */
    int                 def_prio;         /* Default priority for new threads */
    sched_policy_t      def_sched_policy; /* Default policy for new threads */
    /* Process timing */
    bigtime_t           kernel_time;      /* Kernel-side execution time */
    bigtime_t           user_time;        /* User-side execution time */
    /* Process memory */
    aspace_id           aid;              /* Address space id */
    vm_address_space_t  *aspace;          /* Address space */
    /* Internal data structures */
    spinlock_t          lock;             /* Lock for lists access */
    struct thread       *main;            /* Main thread */
    xlist_t             threads;          /* Threads associated with process */
    xlist_t             children;         /* List of child processes */
    list_elem_t         sibling_node;     /* Node in parent process */
    xlist_t             semaphores;       /* Owned semaphores list */
    /* Node of global processes list and tree */
    list_elem_t         procs_list_node;  /* Processes list node */
    avl_tree_node_t     procs_tree_node;  /* Processes tree node */
    /* Hardware-dependend data */
    arch_process_t      arch;             /* Architecture-dependend data */
} process_t;

/* Process states */
enum {
  PROCESS_STATE_NORMAL = 0,  /* Normal state */
  PROCESS_STATE_BIRTH,       /* Process is in creation stage */
  PROCESS_STATE_DEATH        /* Process is in destroying stage */
};

/* Process roles */
enum {
  PROCESS_ROLE_KERNEL = 0,  /* Kernel process. Only one process has this role. */
  PROCESS_ROLE_SERVICE,     /* Process is system service */
  PROCESS_ROLE_USER,        /* Ordinary user process */
  /* Count of process roles */
  PROCESS_ROLES_COUNT
};

/* Process flags */
enum {
  PROCESS_FLAG_NONE        = 0x0,  /* No flags is set */
  PROCESS_FLAG_INTERACTIVE = 0x1   /* Process currently interactive */
};

/* Thread callback descriptor */
typedef struct thread_cbd {
    thread_t *thread;                  /* Thread the callback assigned to */
    void (*func)(struct thread_cbd*);  /* Function to call */
    void *data;                        /* User data */
    list_elem_t list_node;             /* List node */
} thread_cbd_t;


#endif
