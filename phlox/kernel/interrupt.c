/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <phlox/kernel.h>
#include <phlox/ktypes.h>
#include <phlox/kargs.h>
#include <phlox/platform/irq.h>
#include <phlox/errors.h>
#include <phlox/spinlock.h>
#include <phlox/heap.h>
#include <phlox/interrupt.h>

/* interrupt handler entry */
struct hw_int_handler_s {
    struct hw_int_handler_s  *next;    /* pointer to next handler struct */
    hw_int_handler_t         *handler; /* handling routine */
    void                     *data;    /* additional data */
    const char               *name;    /* handler name */
};

/* hardware interrupt vector */
struct hw_int_vector_s {
    struct hw_int_handler_s  *handlers;  /* handlers list */
    spinlock_t               lock;       /* lock for vector modify */
};

/* hardware interrupt vectors */
struct hw_int_vector_s hw_vectors[HW_INTERRUPTS_NUM];


/* init interrupt handling */
uint32 interrupt_init(kernel_args_t *kargs)
{
    /* call architecture specific init */
    arch_interrupt_init(kargs);

    /* init hw_vectors array to zeroes */
    memset(hw_vectors, 0, HW_INTERRUPTS_NUM * sizeof(struct hw_int_vector_s));

    return 0;
}

/* handle hardware interrupt */
uint32 handle_hw_interrupt(uint32 hw_vector)
{
    uint32 flags = INT_FLAGS_NOFLAGS;
    struct hw_int_handler_s *h;

    /* set lock to avoid SMP effects */
    smp_spin_lock(&hw_vectors[hw_vector].lock);

    if(hw_vectors[hw_vector].handlers == NULL) {
        kprint("handle_hw_interrupt(): unhandled interrupt 0x%x\n", hw_vector);
    } else {
        /* get first handler from chain */
        h = hw_vectors[hw_vector].handlers;
        /* call handlers until end of chain reached */
        while(h != NULL) {
            /* OR flags */
            flags |= h->handler(h->data);
            /* step to next */
            h = h->next;
        }
    }

    /* release lock */
    smp_spin_unlock(&hw_vectors[hw_vector].lock);

    /* return flags collected from handlers */
    return flags;
}

/* enable hardware interrupt */
uint32 hw_interrupt_enable(uint32 hw_int)
{
    if(hw_int >= HW_INTERRUPTS_NUM)
        return ERR_INVALID_ARGS;

    /* enable */
    interrupt_enable(IRQS_BASE_VECTOR + hw_int);

    return NO_ERROR;
}

/* disable hardware interrupt */
uint32 hw_interrupt_disable(uint32 hw_int)
{
    if(hw_int >= HW_INTERRUPTS_NUM)
        return ERR_INVALID_ARGS;

    /* disable */
    interrupt_disable(IRQS_BASE_VECTOR + hw_int);

    return NO_ERROR;
}

/* set hardware interrupt handler */
uint32 set_hw_interrupt_handler(uint32 hw_vector, hw_int_handler_t *handler,
                                const char *name, void *data)
{
    struct hw_int_handler_s *h = NULL;
    char *hname = NULL;
    uint32 irqs_state;

    /* ensure that input correct */
    if(!handler || hw_vector >= HW_INTERRUPTS_NUM)
        return ERR_INVALID_ARGS;

    /* allocate memory for handler structure and for its name */
    h = (struct hw_int_handler_s *)kmalloc(sizeof(struct hw_int_handler_s));
    hname  = kstrdup(name);
    /* memory allocated ? */
    if(!h || !hname) {
        kfree_and_null((void **)&h);
        kfree_and_null((void **)&hname);
        return ERR_NO_MEMORY;
    }

    /* init handler structure */
    h->handler = handler;
    h->data    = data;
    h->name    = hname;

    /* lock vector */
    irqs_state = spin_lock_irqsave(&hw_vectors[hw_vector].lock);
    /* link handler to handlers chain */
    h->next = hw_vectors[hw_vector].handlers;
    hw_vectors[hw_vector].handlers = h;
    /* unlock vector */
    spin_unlock_irqrstor(&hw_vectors[hw_vector].lock, irqs_state);

    /* enable hardware interrupt (if not enabled) */
    hw_interrupt_enable(hw_vector);

    return NO_ERROR;
}

/* remove hardware interrupt handler */
uint32 remove_hw_interrupt_handler(uint32 hw_vector, hw_int_handler_t *handler,
                                   void *data)
{
    struct hw_int_handler_s *h, *h_prev = NULL;
    uint32 irqs_state;

    /* check inputs */
    if(!handler || hw_vector >= HW_INTERRUPTS_NUM)
        return ERR_INVALID_ARGS;

    /* lock vector structure before search */
    irqs_state = spin_lock_irqsave(&hw_vectors[hw_vector].lock);

    /* get first handler in chain for start from the beginning */
    h = hw_vectors[hw_vector].handlers;

    /* while end of chain is not reached */
    while(h != NULL) {
        /* see if we match both the handler address and data */
        if(h->handler == handler && h->data == data)
            break;

        /* step to next handler structure in chain */
        h_prev = h;
        h = h->next;
    }

    /* if handler found */
    if(h != NULL) {
        /* unlink it from chain */
        if(h_prev != NULL)
            h_prev->next = h->next; /* if handler is the first in chain... */
        else
            hw_vectors[hw_vector].handlers = h->next; /*... and if not. */
    }

    /* vector can be unlocked here */
    spin_unlock_irqrstor(&hw_vectors[hw_vector].lock, irqs_state);

    if(h != NULL) {
        /* disable interrupt if no handlers in chain */
        if(h_prev == NULL && h->next == NULL)
            hw_interrupt_disable(hw_vector);

        /* free handler's data */
        kfree((char *)h->name);
        kfree(h);
        /* NOTE: h must not be set to NULL, so do not use kfree_and_null here.
         *       see function return.
         */
    }

    /* Once again. h != NULL if it was found and removed from chain of handlers. */
    return (h != NULL) ? NO_ERROR : ERR_INVALID_ARGS;
}
