#### Boot image target
$(call def-target-vars,PHLOXZ,phloxz)

PHLOXZ_TARGET := $(BUILD_DIR)/phloxz

# This is a final target
FINAL_DEP += $(PHLOXZ)

# Additional dependencies
PHLOXZ_DEP = $(MAKEFLOP) $(BOOTBLOCK) $(IMAGEZ)

$(PHLOXZ):
	$(Q)echo "Creating [$(PHLOXZ)]"
	$(Q)$(MAKEFLOP) -v -b 512 $(BOOTBLOCK) $(IMAGEZ) $(PHLOXZ)


#### Boot block target
$(call def-target-vars,BOOTBLOCK,bootblock.bin)

BOOTBLOCK_SRC := \
	$(LOCDIR)/bootblock.S

BOOTBLOCK_LDSCRIPT := $(LOCDIR)/bootblock.ld
BOOTBLOCK_ASFLAGS += $(GLOBAL_ASFLAGS) $(GLOBAL_CFLAGS) $(KERNEL_CFLAGS)

BOOTBLOCK_DEP = $(BOOTBLOCK_LDSCRIPT)

$(BOOTBLOCK):
	$(Q)echo "Creating [$(BOOTBLOCK)]"
	$(Q)$(LD) $(GLOBAL_LDFLAGS) -dN -script=$(BOOTBLOCK_LDSCRIPT) -o $(basename $(BOOTBLOCK)).elf $(BOOTBLOCK_OBJ)
	$(Q)$(OBJCOPY) -O binary $(basename $(BOOTBLOCK)).elf $(BOOTBLOCK)

BOOTBLOCK_clean:
	-$(Q)rm -f $(basename $(BOOTBLOCK)).elf

# Generate explicit rules for boot block source
$(call gen-explicit-s-rule,BOOTBLOCK,$(BUILD_DIR),$(LOCDIR),$(notdir $(BOOTBLOCK_SRC)))


#### Make floppy utility target
$(call def-target-vars,MAKEFLOP,makeflop)

MAKEFLOP_SRC := \
	$(LOCDIR)/makeflop.c

MAKEFLOP_CFLAGS += $(TOOLS_CFLAGS) $(TOOLS_INCLUDES)

$(MAKEFLOP):
	$(Q)echo "Linking [$(MAKEFLOP)]"
	$(Q)$(CC) -o $(MAKEFLOP) $(MAKEFLOP_OBJ)

# Generate explicit rules for makeflop sources
$(call gen-explicit-c-rule,MAKEFLOP,$(BUILD_DIR),$(LOCDIR),$(notdir $(MAKEFLOP_SRC)))


#### Boot image decompression
$(call def-target-vars,BOOTDECOMP,bootdecomp.bin)

BOOTDECOMP_SRC := \
	$(LOCDIR)/bootdecomp.c \
	$(LOCDIR)/inflate.c

BOOTDECOMP_CFLAGS += $(GLOBAL_CFLAGS) $(KERNEL_CFLAGS) $(LIBGCC_INCLUDE) $(INCLUDES)
BOOTDECOMP_LDSCRIPT := $(LOCDIR)/bootdecomp.ld

BOOTDECOMP_DEP = $(BOOTDECOMP_LDSCRIPT) $(LIBSTRING) $(LIBPHLOX)

$(BOOTDECOMP):
	$(Q)echo "Creating [$(BOOTDECOMP)]"
	$(Q)$(LD) $(GLOBAL_LDFLAGS) -L$(LIBGCC_PATH) -dN -script=$(BOOTDECOMP_LDSCRIPT) -o $(basename $(BOOTDECOMP)).elf $(BOOTDECOMP_OBJ) $(LIBPHLOX) $(LIBSTRING) $(LIBGCC)
	$(Q)$(OBJCOPY) -O binary $(basename $(BOOTDECOMP)).elf $(BOOTDECOMP)

BOOTDECOMP_clean:
	-$(Q)rm -f $(basename $(BOOTDECOMP)).elf


# Generate explicit rules for decompressor sources
$(call gen-explicit-c-rule,BOOTDECOMP,$(BUILD_DIR),$(LOCDIR),$(notdir $(BOOTDECOMP_SRC)))


#### Compressed boot file system image
$(call def-target-vars,IMAGEZ,imagez)

IMAGEZ_INI := $(LOCDIR)/bootfs.ini
IMAGEZ_DEP = $(IMAGEZ_INI) $(MAKEBOOTFS) $(BOOTDECOMP) $(KINIT) $(PHLOXK)

$(IMAGEZ):
	$(Q)echo "Creating [$(IMAGEZ)]"
	$(Q)$(MAKEBOOTFS) $(IMAGEZ_INI) $(dir $(IMAGEZ))image
	$(Q)gzip -f -9 $(dir $(IMAGEZ))image
	$(Q)cat $(BOOTDECOMP) $(dir $(IMAGEZ))image.gz > $(IMAGEZ)

IMAGEZ_clean:
	-$(Q)rm -f $(dir $(IMAGEZ))image.gz


#### Kernel init stage
$(call def-target-vars,KINIT,kinit)

KINIT_SRC := \
	$(LOCDIR)/kinit.c

KINIT_CFLAGS += $(GLOBAL_CFLAGS) $(KERNEL_CFLAGS) $(LIBGCC_INCLUDE) $(INCLUDES)
KINIT_LDSCRIPT := $(LOCDIR)/kinit.ld

KINIT_DEP = $(KINIT_LDSCRIPT) $(LIBSTRING) $(LIBPHLOX)

$(KINIT):
	$(Q)echo "Linking [$(KINIT)]"
	$(Q)$(LD) $(GLOBAL_LDFLAGS) -L$(LIBGCC_PATH) -dN -script=$(KINIT_LDSCRIPT) -o $(KINIT) $(KINIT_OBJ) $(LIBPHLOX) $(LIBSTRING) $(LIBGCC)

# Generate explicit rules for kinit
$(call gen-explicit-c-rule,KINIT,$(BUILD_DIR),$(LOCDIR),$(notdir $(KINIT_SRC)))
