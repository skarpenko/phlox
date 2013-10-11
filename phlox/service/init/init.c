/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/

#include <app/syslib.h>

int main(int argc, char **argv)
{
    char *welcome = "\n======================================================\n"
        "Hello from user space!\n";
    char *tests = "test/test_main.elf";

    /* welcome message */
    klog_printf(welcome);

    klog_printf("Running tests...\n");

    if(service_load(tests))
        klog_printf("FAILED to load %s!\n", tests);

    while(1)
        ;

    return 0;
}
