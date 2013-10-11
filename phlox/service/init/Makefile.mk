$(call def-target-vars,INIT,init.elf)

INIT_SRC += \
	$(LOCDIR)/init.c

INIT_DEP = $(LIBPHLOX)

INIT_CFLAGS += $(GLOBAL_CFLAGS) $(USER_CFLAGS) $(INCLUDES)

$(INIT):
	$(Q)echo "Linking [$(INIT)]"
	$(Q)$(LD) $(GLOBAL_LDFLAGS) -dN -script=$(SERVICE_LDSCRIPT) -o $(INIT) $(INIT_OBJ) $(LIBPHLOX)
