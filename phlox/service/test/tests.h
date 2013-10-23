/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _SYS_TESTS_H_
#define _SYS_TESTS_H_


#define TEST0_NAME "Thread creation"
extern int test0(void);

#define TEST1_NAME "Synchronizing threads with semaphore"
extern int test1(void);

#define TEST2_NAME "Thread suspend and resume"
extern int test2(void);

#define TEST3_NAME "Threads creation stress test"
extern int test3(void);

#define TEST4_NAME "Semaphore signaling"
extern int test4(void);

#define TEST5_NAME "Virtual memory allocation"
extern int test5(void);

#define TEST6_NAME "Virtual memory allocation stress test"
extern int test6(void);


#endif
