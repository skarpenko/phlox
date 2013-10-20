/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/

#include <app/syslib.h>
#include "tests.h"


/* Test descriptor */
struct test_spec {
    const char *name;          /* Test name */
    const int  skip;           /* Skip test if !=0 */
    int        (*func)(void);  /* Test function */
    int         result;        /* Result, non zero value denotes success */
};

/* Tests table */
struct test_spec tests_table[] = {
    {
        .name   = "Thread creation                      ",
        .skip   = 0,
        .func   = thread_test_creation,
        .result = 0
    },
    {
        .name   = "Semaphore signaling                  ",
        .skip   = 0,
        .func   = sem_test_signaling,
        .result = 0
    },
    {
        .name   = "Virtual memory allocation            ",
        .skip   = 0,
        .func   = mem_test_virtmem_alloc,
        .result = 0
    },
    {
        .name   = "Synchronizing threads with semaphore ",
        .skip   = 0,
        .func   = thread_test_sem_sync,
        .result = 0
    },
    {
        .name   = "Thread suspend and resume            ",
        .skip   = 0,
        .func   = thread_test_suspend_resume,
        .result = 0
    },
    {
        .name   = "Threads creation stress test         ",
        .skip   = 1,
        .func   = thread_test_creation_stress,
        .result = 0
    },
    {
        .name   = "Virtual memory allocation stress test",
        .skip   = 0,
        .func   = mem_test_virtmem_alloc_stress,
        .result = 0
    },
};
const int nr_tests = sizeof(tests_table) / sizeof(tests_table[0]);

/* Run all tests from the table */
static void run_tests()
{
    int i;
    for(i = 0; i < nr_tests; ++i) {
        klog_printf("%d. %s ... ", i, tests_table[i].name);
        /* ignore tests marked to skip */
        if(tests_table[i].skip) {
            klog_printf("SKIPPED     (%d of %d)\n", i+1, nr_tests);
            continue;
        }

        /* run test function */
        tests_table[i].result = tests_table[i].func();
        klog_printf("%s      (%d of %d)\n",
                tests_table[i].result ? "PASSED" : "FAILED", i+1, nr_tests);
    }
}

/* Print final results */
static void print_summary()
{
    int passed = 0;
    int failed = 0;
    int skipped = 0;
    int i;

    for(i = 0; i < nr_tests; ++i) {
        if(tests_table[i].skip)
            ++skipped;
        else if(tests_table[i].result)
            ++passed;
        else
            ++failed;
    }
    klog_printf("\n================================= SUMMARY ==================================\n");
    klog_printf("PASSED:  %d\nSKIPPED: %d\nFAILED:  %d\n", passed, skipped, failed);
    klog_printf("============================================================================\n");
}

/* signal semaphore created at init service */
static void signal_completion()
{
    sem_id sid;

    /* get global semaphore created at init service */
    sid = semaphore_get_by_name("sem_test_compl");
    if(sid == INVALID_SEMID) {
        klog_printf("FAILED to signal completion.\n");
        return;
    }

    /* signal tests completion */
    semaphore_up(sid, 1);
}

/* Main */
int main(int argc, char **argv)
{
    klog_printf("\n============================== RUNNING TESTS ===============================\n");

    /* run tests from the table */
    run_tests();

    /* print summary */
    print_summary();

    /* report to init service */
    signal_completion();

    /* suspend execution as process termination is not implemented yet */
    thread_suspend_current();

    while(1)
        ;

    return 0;
}
