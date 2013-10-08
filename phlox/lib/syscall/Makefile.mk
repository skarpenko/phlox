$(call def-target-vars,LIBSYSCALL,libsyscall.a)

LIBSYSCALL_SRC +=    \
	$(LOCDIR)/syscall.c

LIBSYSCALL_CFLAGS += $(GLOBAL_CFLAGS) $(USER_CFLAGS) $(INCLUDES)

# Include arch-specific
-include $(LOCDIR)/arch/$(ARCH)/Makefile.arch

$(LIBSYSCALL):
	$(Q)echo "Creating [$(LIBSYSCALL)]"
	$(Q)$(AR) r $(LIBSYSCALL) $(LIBSYSCALL_OBJ)
