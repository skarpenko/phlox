/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/

.text

.extern  __do_global_ctors
.extern  __startup
.extern  __do_global_dtors
.extern  __terminate


.global _start
.type _start,@function
.align 8
_start:
/*
 * Stack was set up by kernel, so there is no need to setup it here
*/

    /* Call global constructors */
    call __do_global_ctors
    /* Call to main program entry */
    call __startup
    pushl %eax /* store return value as argument for __terminate */
    /* Call global destructors */
    call __do_global_dtors
    /* Terminate process */
    call __terminate
    /* It is not expected that __terminate exit, but remove it's arg from
     * stack.
     */
    addl $4, %esp
    hlt
    hlt
    hlt
    hlt
/* END */
