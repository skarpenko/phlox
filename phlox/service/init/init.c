/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#include <app/syslib.h>


int main(int argc, char **argv)
{
    char *welcome = "\nInit service started.\n";
    char *tests = "test/test_main.elf";
    sem_id sid;

    /* welcome message */
    klog_printf(welcome);

    /* create completion semaphore */
    sid = semaphore_create("sem_test_compl", 1, 0);
    if(sid == INVALID_SEMID)
        klog_printf("FAILED to create 'sem_test_compl' semaphore!\n");

    /* load tests binary */
    if(service_load(tests))
        klog_printf("FAILED to load %s!\n", tests);

    /* wait for tests completion will be reported */
    if(sid != INVALID_SEMID) {
        semaphore_down(sid, 1);
        klog_printf("Tests execution completed!\n");
    }

    /* suspend execution */
    thread_suspend_current();

    while(1)
        ;

    return 0;
}
