/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/


typedef void (*ctor)();  /* ctor func */
typedef void (*dtor)();  /* dtor func */


void __do_global_ctors(void);
void __do_global_dtors(void);
int __startup(void);
void __terminate(int code);


/* called during startup to call global constructors */
void __do_global_ctors(void)
{
    extern int __ctor_list;
    int *c = &__ctor_list;

    while (*c) {
        ctor ct = (ctor)*c;
        (ct)();
        ++c;
    }
}

/* called during termination to call global destructors */
void __do_global_dtors(void)
{
    extern int __dtor_list;
    int *c = &__dtor_list;
    int *cp = c;

    while (*c)
        ++c;

    --c;

    /* call destructurs in reverse order */
    while (c > cp) {
        dtor dt = (dtor)*c;
        (*dt)();
        --c;
    }
}

/* Main entry point */
int __startup(void)
{
    /* TODO: */
    extern int main(int argc, char **argv);
    static char* argv[1] = { "serv" };
    return main(1, argv);
}

/* Process termination */
void __terminate(int code)
{
    /* TODO: */
    while(1)
        ;
}
