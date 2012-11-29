LIBSTRING_SRC_DIR := $(GET_LOCAL_DIR)
LIBSTRING_OUT_DIR := $(BUILD_DIR)/$(LIBSTRING_SRC_DIR)
$(shell mkdir -p $(LIBSTRING_OUT_DIR))

LIBSTRING := $(LIBSTRING_OUT_DIR)/$(LIBSTRING)

LIBSTRING_O_FILES := \
	bcopy.o \
	bzero.o \
	memchr.o \
	memcmp.o \
	memcpy.o \
	memmove.o \
	memscan.o \
	memset.o \
	strcat.o \
	strchr.o \
	strcmp.o \
	strcoll.o \
	strcpy.o \
	strlcat.o \
	strlcpy.o \
	strlen.o \
	strncat.o \
	strncmp.o \
	strncpy.o \
	strnicmp.o \
	strnlen.o \
	strpbrk.o \
	strrchr.o \
	strspn.o \
	strstr.o \
	strtok.o \
	strxfrm.o

LIBSTRING_O_FILES:=$(addprefix $(LIBSTRING_OUT_DIR)/,$(LIBSTRING_O_FILES))

include $(LIBSTRING_SRC_DIR)/arch/$(ARCH)/makefile

ALL_OBJ += $(LIBSTRING_O_FILES)

$(LIBSTRING): $(LIBSTRING_O_FILES)
	$(AR) r $(LIBSTRING) $(LIBSTRING_O_FILES)


$(LIBSTRING_OUT_DIR)/%.o: $(LIBSTRING_SRC_DIR)/%.c
	$(CC) -c $< $(GLOBAL_CFLAGS) $(KERNEL_CFLAGS) $(INCLUDES) -o $@
