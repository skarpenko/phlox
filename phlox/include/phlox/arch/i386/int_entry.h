/*
* Copyright 2007-2013, Stepan V.Karpenko. All rights reserved.
* Distributed under the terms of the PhloxOS License.
*/
#ifndef _PHLOX_ARCH_I386_INT_ENTRY_H_
#define _PHLOX_ARCH_I386_INT_ENTRY_H_

/* set of default interrupt entries */
void interrupt0(void);   void interrupt1(void);   void interrupt2(void);
void interrupt3(void);   void interrupt4(void);   void interrupt5(void);
void interrupt6(void);   void interrupt7(void);   void interrupt8(void);
void interrupt9(void);   void interrupt10(void);  void interrupt11(void);
void interrupt12(void);  void interrupt13(void);  void interrupt14(void);
void interrupt16(void);  void interrupt17(void);  void interrupt18(void);
void interrupt19(void);  void interrupt32(void);  void interrupt33(void);
void interrupt34(void);  void interrupt35(void);  void interrupt36(void);
void interrupt37(void);  void interrupt38(void);  void interrupt39(void);
void interrupt40(void);  void interrupt41(void);  void interrupt42(void);
void interrupt43(void);  void interrupt44(void);  void interrupt45(void);
void interrupt46(void);  void interrupt47(void);

/* dummy interrupt handler */
void dummy_interrupt();

/* system call entry */
void system_call();

#endif
