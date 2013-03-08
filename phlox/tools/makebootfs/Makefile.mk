$(call def-target-vars,MAKEBOOTFS,makebootfs)

MAKEBOOTFS_SRC += \
	$(LOCDIR)/makebootfs.c

MAKEBOOTFS_CFLAGS += $(TOOLS_CFLAGS) $(TOOLS_INCLUDES)

$(MAKEBOOTFS):
	$(Q)echo "Linking [$(MAKEBOOTFS)]"
	$(Q)$(CC) -o $(MAKEBOOTFS) $(MAKEBOOTFS_OBJ)
