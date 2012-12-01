KERNEL_VM_SRC_DIR := $(GET_LOCAL_DIR)
KERNEL_VM_OUT_DIR := $(KERNEL_OUT_DIR)/vm
$(shell mkdir -p $(KERNEL_VM_OUT_DIR))

KERNEL_VM_O_FILES += \
	vm.o \
	vm_page.o \
	vm_page_mapper.o \
	vm_address_space.o \
	vm_object.o

KERNEL_VM_O_FILES := $(addprefix $(KERNEL_VM_OUT_DIR)/,$(KERNEL_VM_O_FILES))

KERNEL_O_FILES += $(KERNEL_VM_O_FILES)


$(KERNEL_VM_OUT_DIR)/%.o: $(KERNEL_VM_SRC_DIR)/%.c
	$(CC) -c $< $(GLOBAL_CFLAGS) $(KERNEL_CFLAGS) $(INCLUDES) -o $@
