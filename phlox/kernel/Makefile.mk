$(call def-target-vars,PHLOXK,phloxk)

PHLOXK_TOPDIR := $(LOCDIR)

PHLOXK_SRC +=                 \
	$(LOCDIR)/main.c      \
	$(LOCDIR)/spinlock.c  \
	$(LOCDIR)/processor.c \
	$(LOCDIR)/heap.c      \
	$(LOCDIR)/machine.c   \
	$(LOCDIR)/interrupt.c \
	$(LOCDIR)/timer.c     \
	$(LOCDIR)/misc.c      \
	$(LOCDIR)/klog.c      \
	$(LOCDIR)/debug.c     \
	$(LOCDIR)/process.c   \
	$(LOCDIR)/scheduler.c \
	$(LOCDIR)/thread.c    \
	$(LOCDIR)/sem.c       \
	$(LOCDIR)/elf_file.c  \
	$(LOCDIR)/syscall.c   \
	$(LOCDIR)/imgload.c   \
	$(LOCDIR)/kutil.c

PHLOXK_CFLAGS += $(GLOBAL_CFLAGS) $(KERNEL_CFLAGS) $(INCLUDES)

# Include subdirs
include $(call include-makefiles,Makefile.phloxk,$(LOCDIR))
# Include architecture specifics
include $(PHLOXK_TOPDIR)/arch/$(ARCH)/Makefile.arch
# Include platform specifics
include $(PHLOXK_TOPDIR)/platform/$(PLATFORM)/Makefile.platform
