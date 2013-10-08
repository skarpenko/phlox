/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/

#include <app/syscall.h>

int main(int argc, char **argv)
{
    char str[] = "\n=========================================================\n"
        "Hello from user space!\n";

    sys_klog_puts(str, sizeof(str));

    while(1)
        ;

    return 0;
}
