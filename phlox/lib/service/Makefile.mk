$(call def-target-vars,LIBSERVICE,libservice.a)

LIBSERVICE_SRC +=    \
	$(LOCDIR)/startup.c

LIBSERVICE_CFLAGS += $(GLOBAL_CFLAGS) $(USER_CFLAGS) $(INCLUDES)

# Include arch-specific
-include $(LOCDIR)/arch/$(ARCH)/Makefile.arch

$(LIBSERVICE):
	$(Q)echo "Creating [$(LIBSERVICE)]"
	$(Q)$(AR) r $(LIBSERVICE) $(LIBSERVICE_OBJ)
