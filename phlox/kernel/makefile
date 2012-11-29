KERNEL_SRC_DIR := $(GET_LOCAL_DIR)
KERNEL_OUT_DIR := $(BUILD_DIR)/$(KERNEL_SRC_DIR)
$(shell mkdir -p $(KERNEL_OUT_DIR))

KERNEL := $(KERNEL_OUT_DIR)/$(KERNEL)

KERNEL_O_FILES := \
	main.o \
	spinlock.o \
	processor.o \
	heap.o \
	machine.o \
	interrupt.o \
	misc.o

KERNEL_O_FILES:=$(addprefix $(KERNEL_OUT_DIR)/,$(KERNEL_O_FILES))

include $(KERNEL_SRC_DIR)/vm/makefile
include $(KERNEL_SRC_DIR)/util/makefile
include $(KERNEL_SRC_DIR)/platform/$(PLATFORM)/makefile
# !! this include must be the last 'cause includes final target !!
include $(KERNEL_SRC_DIR)/arch/$(ARCH)/makefile

ALL_OBJ += $(KERNEL_O_FILES)


$(KERNEL_OUT_DIR)/%.o: $(KERNEL_SRC_DIR)/%.c
	$(CC) -c $< $(GLOBAL_CFLAGS) $(KERNEL_CFLAGS) $(INCLUDES) -o $@
