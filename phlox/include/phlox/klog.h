/*
* Copyright 2007-2008, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_KLOG_H_
#define _PHLOX_KLOG_H_

#include <phlox/ktypes.h>
#include <phlox/kargs.h>


/*
 * Kernel log module initialization.
 * Called during kernel start up process.
 */
status_t klog_init(kernel_args_t *kargs);

/*
 * Get kernel log columns number
 */
uint klog_get_cols_num(void);

/*
 * Get kernel log rows number
 */
uint klog_get_rows_num(void);

/*
 * Get row counter. Counter increments every
 * time new log row prepared.
 */
uint klog_get_row_counter(void);

/*
 * Get least row counter. Least row counter
 * always matching bottom row.
 */
uint klog_get_least_row_counter(void);

/*
 * Get kernel log top row index
 * (top row is a current row)
 */
uint klog_get_top_index(void);

/*
 * Get kernel log bottom row index
 * (bottom row is a last row in the log)
 */
uint klog_get_bottom_index(void);

/*
 * Get row by absolute index
 */
status_t klog_get_row_abs(uint index, char *row);

/*
 * Get row by top-based index.
 * (index = 0 is a top, index = 1 is a previous row and so on)
 */
status_t klog_get_row(uint index, char *row);

/*
 * Get previous row
 */
status_t klog_get_previous_row(char *row);

/*
 * Get current row
 */
status_t klog_get_current_row(char *row);

/*
 * Get last row in a log
 */
status_t klog_get_last_row(char *row);

/*
 * Get row by row counter.
 * Returns counter for next row or passed
 * counter if no new rows was added into log.
 */
uint klog_get_new_row(uint counter, char *row);

/*
 * Put string into kernel log
 */
void klog_puts(char *str);

/*
 * Clear kernel log
 */
void klog_clear(void);


#endif
