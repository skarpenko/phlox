/*
* Copyright 2007-2010, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_THREAD_PRIVATE_H_
#define _PHLOX_THREAD_PRIVATE_H_

/*******************************************************************
 * THREADING INTERNALS. DO NOT USE!
 *******************************************************************/

/* Thread kernel-side stack size */
#define THREAD_KSTACK_SIZE  (KERNEL_STACK_SIZE*PAGE_SIZE)
/* Thread kernel-side stack warning value */
#define THREAD_KSTACK_WARN  (THREAD_KSTACK_SIZE/8)
/*
 * Thread kernel-side stack layout
 *
 * |-------| <- Top = Base + THREAD_KSTACK_SIZE - 1
 * |-------|
 * |-------|
 * |-------|
 * |-------|
 * |-------|
 * |-------| <- Danger stack usage barrier (Base + THREAD_KSTACK_WARN)
 * |--Ptr--| <- Base
 *
 * Ptr - pointer to thread_t structure.
*/

/* Per thread time quanta params in milliseconds */
#define THREAD_DEFAULT_QUANTA      60  /* Default quanta size */
#define THREAD_MINIMUM_QUANTA      20  /* Minimum quanta size */
#define THREAD_QUANTA_GRANULARITY  10  /* Quanta granularity  */

/* Priorities constants */
#define THREAD_NUM_PRIORITY_LEVELS  128
#define THREAD_LOWEST_PIORITY         0
#define THREAD_HIGHEST_PRIORITY     (THREAD_NUM_PRIORITY_LEVELS-1)

/* Additional priority levels added to thread depending on process role */
#define PROCESS_ROLE_PRIORITY_SHIFT_KERNEL   8
#define PROCESS_ROLE_PRIORITY_SHIFT_SERVICE  4
#define PROCESS_ROLE_PRIORITY_SHIFT_USER     0


/*
 * Per CPU threading initialization stages.
 * Called within threading init.
*/
status_t threading_init_per_cpu(kernel_args_t *kargs, uint curr_cpu);

/*
 * Continue execution of current instructions flow as idle thread.
 * Used during system startup.
*/
status_t thread_continue_as_idle(kernel_args_t *kargs, uint curr_cpu);

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
 * Add idle thread for specified CPU.
 * Called during threading subsystem init.
*/
void sched_add_idle_thread(thread_t *thread, uint cpu);

/*
 * Add thread for scheduling.
*/
void sched_add_thread(thread_t *thread);

/*
 * Remove thread from scheduling.
*/
void sched_remove_thread(thread_t *thread);

/*
 * Called from thread stub and inside scheduler
 * for performing last steps of context switch.
*/
void sched_complete_context_switch(void);

/*
 * Create kernel process.
 * Called only once during system init stage.
*/
proc_id proc_create_kernel_process(const char* name);

/*
 * Attach new thread to process.
 * Process lock acquired inside.
*/
void proc_attach_thread(process_t *proc, thread_t *thread);

/*
 * Detach thread from process.
 * Process lock acquired inside.
*/
void proc_detach_thread(process_t *proc, thread_t *thread);


#endif
