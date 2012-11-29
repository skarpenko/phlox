/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <string.h>
#include <phlox/errors.h>
#include <phlox/spinlock.h>
#include <phlox/klog.h>

/* Kernel log dimensions */
#define KLOG_N_ROWS  128   /* Rows number    */
#define KLOG_N_COLS   64   /* Columns number */
#define KLOG_LOGBOOK_SIZE  (KLOG_N_ROWS * KLOG_N_COLS)

/* Variables */
spinlock_t klog_lock;  /* Access lock */
uint row_counter;      /* Cyclic row counter */
int top;               /* Top row (points to current row) */
int bottom;            /* Bottom row (points to last row) */
int cursor;            /* Cursor position */
char logbook[KLOG_N_ROWS][KLOG_N_COLS];  /* Logbook */


/** Locally used routines **/

/* clear row */
static inline void _clear_row(uint index)
{
    memset(&logbook[index][0], 0, KLOG_N_COLS);
}

/* scroll rows up or down */
static inline void _scroll_rows(int delta)
{
    /* recompute indexes */
    top = (top + delta + KLOG_N_ROWS) % KLOG_N_ROWS;
    bottom = (top + 1 + KLOG_N_ROWS) % KLOG_N_ROWS;
}

/* get top-based row index */
static inline uint _get_row_index(uint rel_index)
{
    return (top - rel_index + KLOG_N_ROWS) % KLOG_N_ROWS;
}


/** Public routines **/

/* module init */
status_t klog_init(kernel_args_t *kargs)
{
    /* init spinlock */
    spin_init(&klog_lock);

    /* init variables */
    row_counter = 0;
    cursor = 0;
    top = 0;
    bottom = 1;

    /* clear logbook */
    memset(logbook, 0, KLOG_LOGBOOK_SIZE);

    return NO_ERROR;
}

/* returns columns count */
uint klog_get_cols_num(void)
{
    return KLOG_N_COLS;
}

/* returns rows count */
uint klog_get_rows_num(void)
{
    return KLOG_N_ROWS;
}

/* returns cyclic rows counter */
uint klog_get_row_counter(void)
{
    uint irqs_state;
    uint value;

    /* acquire access lock */
    irqs_state = spin_lock_irqsave(&klog_lock);

    /* copy value to temporary variable */
    value = row_counter;

    /* release lock */
    spin_unlock_irqrstor(&klog_lock, irqs_state);

    /* return value to caller */
    return value;
}

/* returns top index */
uint klog_get_top_index(void)
{
    uint irqs_state;
    uint value;

    /* acquire lock */
    irqs_state = spin_lock_irqsave(&klog_lock);

    /* copy value */
    value = top;

    /* release lock */
    spin_unlock_irqrstor(&klog_lock, irqs_state);

    /* return value to caller */
    return value;
}

/* returns bottom index */
uint klog_get_bottom_index(void)
{
    uint irqs_state;
    uint value;

    /* acquire lock */
    irqs_state = spin_lock_irqsave(&klog_lock);

    /* copy value */
    value = bottom;

    /* release lock */
    spin_unlock_irqrstor(&klog_lock, irqs_state);

    /* return value to caller */
    return value;
}

/* get row by absolute index */
status_t klog_get_row_abs(uint index, char *row)
{
    uint irqs_state;

    /* check arguments */
    if(!row || index >= KLOG_N_ROWS)
        return ERR_INVALID_ARGS;
    
    /* acquire lock */
    irqs_state = spin_lock_irqsave(&klog_lock);

    /* copy requested row */
    memcpy(row, &logbook[index][0], KLOG_N_COLS);

    /* release lock */
    spin_unlock_irqrstor(&klog_lock, irqs_state);
   
    return NO_ERROR;
}

/* get row by top-based index */
status_t klog_get_row(uint index, char *row)
{
    uint irqs_state;

    /* check arguments */
    if(!row || index >= KLOG_N_ROWS)
        return ERR_INVALID_ARGS;
    
    /* acquire access lock */
    irqs_state = spin_lock_irqsave(&klog_lock);

    /* compute absolute index */
    index = _get_row_index(index);

    /* copy row to caller */
    memcpy(row, &logbook[index][0], KLOG_N_COLS);

    /* release lock */
    spin_unlock_irqrstor(&klog_lock, irqs_state);
   
    return NO_ERROR;
}

/* get previous row */
status_t klog_get_previous_row(char *row)
{
    return klog_get_row(1, row);
}

/* get current row */
status_t klog_get_current_row(char *row)
{
    return klog_get_row_abs(top, row);
}

/* get last row */
status_t klog_get_last_row(char *row)
{
    return klog_get_row_abs(bottom, row);
}

/* put string into logbook */
void klog_puts(char *str)
{
    uint len = strlen(str);
    uint irqs_state;
    int  new_row = 0; /* new row needed flag */
    uint i;
    
    /* acquire access lock */
    irqs_state = spin_lock_irqsave(&klog_lock);

    /* walk through source string */
    for(i = 0; i < len; i++) {
        /* check for escape chars */
        switch(str[i]) {
            /* new line */
            case '\n':
              new_row = 1;
              break;

            /* carriage return */
            case '\r':
              cursor = 0;
              break;

            /* form feed */
            case '\f':
              /* scroll rows */
              _scroll_rows(1);
              /* update rows counter */
              row_counter++;
              /* clear new row */
              _clear_row(top);
              /* if needed put spaces before cursor */
              if(cursor)
                memset(&logbook[top][0], 32, cursor);
              break;

            /* ordinary characters */
            default:
              /* put char into logbook */
              logbook[top][cursor] = str[i];
              /* update cursor and check for new line need */
              if(++cursor >= KLOG_N_COLS)
                new_row = 1;
              break;
        }

        /* if new line needed */
        if (new_row) {
            new_row = 0; /* reset flag */
            /* scroll rows */
            _scroll_rows(1);
            /* update row counter */
            row_counter++;
            /* clear new row */
            _clear_row(top);
            /* reset cursor */
            cursor = 0;
        }
    }
    
    /* release lock */
    spin_unlock_irqrstor(&klog_lock, irqs_state);
}

/* clear logbook */
void klog_clear(void)
{
    uint irqs_state;

    /* acquire lock */
    irqs_state = spin_lock_irqsave(&klog_lock);

    /* reset cursor */
    cursor = 0;
    /* clear memory */
    memset(logbook, 0, KLOG_LOGBOOK_SIZE);

    /* release lock */
    spin_unlock_irqrstor(&klog_lock, irqs_state);
}
