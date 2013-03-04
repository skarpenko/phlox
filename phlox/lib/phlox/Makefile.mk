$(call def-target-vars,LIBPHLOX,libphlox.a)

LIBPHLOX_SRC +=              \
	$(LOCDIR)/bootfs.c   \
	$(LOCDIR)/ctype.c    \
	$(LOCDIR)/vsprintf.c

LIBPHLOX_CFLAGS += $(GLOBAL_CFLAGS) $(KERNEL_CFLAGS) $(INCLUDES)

$(LIBPHLOX):
	$(Q)echo "Assembling [$(LIBPHLOX)]..."
	$(Q)$(AR) r $(LIBPHLOX) $(LIBPHLOX_OBJ)
