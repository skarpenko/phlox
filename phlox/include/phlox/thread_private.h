/*
* Copyright 2007-2009, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_THREAD_PRIVATE_H_
#define _PHLOX_THREAD_PRIVATE_H_

/*******************************************************************
 * THREADING INTERNALS. DO NOT USE!
 *******************************************************************/

/*
 * Per CPU threading initialization stages.
 * Called within threading init.
*/
status_t threading_init_per_cpu(kernel_args_t *kargs, uint curr_cpu);

/*
 * Process module init.
 * Called only during threading init stage!
*/
status_t process_init(kernel_args_t *kargs);

/*
 * Process module per CPU init.
 * Called only during threading init stage!
*/
status_t process_init_per_cpu(kernel_args_t *kargs, uint curr_cpu);

/*
 * Scheduling engine init.
 * Called only within threading init stages!
*/
status_t scheduler_init(kernel_args_t *kargs);

/*
 * Per CPU scheduler init.
 * Called only during threading init!
*/
status_t scheduler_init_per_cpu(kernel_args_t *kargs, uint curr_cpu);

/*
 * Create kernel process.
 * Called only once during system init stage.
*/
proc_id proc_create_kernel_process(const char* name);


#endif
