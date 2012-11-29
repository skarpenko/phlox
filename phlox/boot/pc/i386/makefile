BOOT_SRC_DIR := $(GET_LOCAL_DIR)

# targets
PHLOXZ     := $(BUILD_DIR)/$(PHLOXZ)
BOOTBLOCK  := $(BOOT_OUT_DIR)/bootblock.bin
MAKEFLOP   := $(BOOT_OUT_DIR)/makeflop
BOOTDECOMP := $(BOOT_OUT_DIR)/bootdecomp.bin
IMAGEZ     := $(BOOT_OUT_DIR)/imagez
KINIT      := $(BOOT_OUT_DIR)/kinit

$(PHLOXZ): $(MAKEFLOP) $(BOOTBLOCK) $(IMAGEZ)
	$(MAKEFLOP) -v -b 512 $(BOOTBLOCK) $(IMAGEZ) $(PHLOXZ)

$(MAKEFLOP): $(BOOT_SRC_DIR)/makeflop.c
	$(CC) -O2 $(TOOLS_INCLUDES) -o $(MAKEFLOP) $(BOOT_SRC_DIR)/makeflop.c

BOOTBLOCK_O_FILES := \
	bootblock.o

BOOTBLOCK_O_FILES:=$(addprefix $(BOOT_OUT_DIR)/,$(BOOTBLOCK_O_FILES))

ALL_OBJ += $(BOOTBLOCK_O_FILES)

ALL_OTHER_INTERM += $(BOOT_OUT_DIR)/bootblock.elf

$(BOOTBLOCK): $(BOOTBLOCK_O_FILES)  $(BOOT_SRC_DIR)/bootblock.ld
	$(LD) $(GLOBAL_LDFLAGS) -dN -script=$(BOOT_SRC_DIR)/bootblock.ld -o $(BOOT_OUT_DIR)/bootblock.elf $(BOOTBLOCK_O_FILES)
	$(OBJCOPY) -O binary $(BOOT_OUT_DIR)/bootblock.elf $(BOOT_OUT_DIR)/bootblock.bin

ALL_OTHER_INTERM += $(BOOT_OUT_DIR)/image

$(IMAGEZ): $(BOOT_SRC_DIR)/bootfs.ini $(MAKEBOOTFS) $(BOOTDECOMP) $(KINIT) $(KERNEL)
	$(MAKEBOOTFS) $(BOOT_SRC_DIR)/bootfs.ini $(BOOT_OUT_DIR)/image
	gzip -f -9 $(BOOT_OUT_DIR)/image
	cat $(BOOTDECOMP) $(BOOT_OUT_DIR)/image.gz > $(BOOT_OUT_DIR)/imagez

BOOTDECOMP_O_FILES := \
	bootdecomp.o \
	inflate.o

BOOTDECOMP_O_FILES:=$(addprefix $(BOOT_OUT_DIR)/,$(BOOTDECOMP_O_FILES))

ALL_OBJ += $(BOOTDECOMP_O_FILES)

ALL_OTHER_INTERM += $(BOOT_OUT_DIR)/bootdecomp.elf

$(BOOTDECOMP): $(BOOT_SRC_DIR)/bootdecomp.ld $(BOOTDECOMP_O_FILES) $(LIBSTRING) $(LIBPHLOX)
	$(LD) $(GLOBAL_LDFLAGS) -L$(LIBGCC_PATH) -dN -script=$(BOOT_SRC_DIR)/bootdecomp.ld -o $(BOOT_OUT_DIR)/bootdecomp.elf $(BOOTDECOMP_O_FILES) $(LIBPHLOX) $(LIBSTRING) $(LIBGCC)
	$(OBJCOPY) -O binary $(BOOT_OUT_DIR)/bootdecomp.elf $(BOOTDECOMP)

KINIT_O_FILES := \
	kinit.o

KINIT_O_FILES:=$(addprefix $(BOOT_OUT_DIR)/,$(KINIT_O_FILES))

ALL_OBJ += $(KINIT_O_FILES)

$(KINIT): $(BOOT_SRC_DIR)/kinit.ld $(KINIT_O_FILES) $(LIBSTRING) $(LIBPHLOX)
	$(LD) $(GLOBAL_LDFLAGS) -L$(LIBGCC_PATH) -dN -script=$(BOOT_SRC_DIR)/kinit.ld -o $(KINIT) $(KINIT_O_FILES) $(LIBPHLOX) $(LIBSTRING) $(LIBGCC)


$(BOOT_OUT_DIR)/%.o: $(BOOT_SRC_DIR)/%.c
	$(CC) -c $< $(GLOBAL_CFLAGS) $(KERNEL_CFLAGS) $(LIBGCC_INCLUDE) $(INCLUDES) -o $@

$(BOOT_OUT_DIR)/%.o: $(BOOT_SRC_DIR)/%.S
	$(CC) -c $< $(GLOBAL_ASFLAGS) $(GLOBAL_CFLAGS) $(KERNEL_CFLAGS) -o $@
