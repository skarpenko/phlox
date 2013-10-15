/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _SYS_TESTS_H_
#define _SYS_TESTS_H_


/* Thread tests */
int thread_test_creation(void);
int thread_test_sem_sync(void);

/* Semaphore tests */
int sem_test_signaling(void);

/* Memory allocation tests */
int mem_test_virtmem_alloc(void);


#endif
