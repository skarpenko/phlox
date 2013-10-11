$(call def-target-vars,LIBPHLOX,libphlox.a)

LIBPHLOX_SRC +=              \
	$(LOCDIR)/bootfs.c   \
	$(LOCDIR)/ctype.c    \
	$(LOCDIR)/vsprintf.c \
	$(LOCDIR)/syscall.c  \
	$(LOCDIR)/syslib.c   \
	$(LOCDIR)/startup.c

LIBPHLOX_CFLAGS += $(GLOBAL_CFLAGS) $(INCLUDES)

# Include arch-specific
-include $(LOCDIR)/arch/$(ARCH)/Makefile.arch

$(LIBPHLOX):
	$(Q)echo "Creating [$(LIBPHLOX)]"
	$(Q)$(AR) r $(LIBPHLOX) $(LIBPHLOX_OBJ)
