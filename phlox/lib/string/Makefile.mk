$(call def-target-vars,LIBSTRING,libstring.a)

LIBSTRING_SRC +=    \
	$(LOCDIR)/bcopy.c     \
	$(LOCDIR)/bzero.c     \
	$(LOCDIR)/memchr.c    \
	$(LOCDIR)/memcmp.c    \
	$(LOCDIR)/memcpy.c    \
	$(LOCDIR)/memmove.c   \
	$(LOCDIR)/memscan.c   \
	$(LOCDIR)/memset.c    \
	$(LOCDIR)/strcat.c    \
	$(LOCDIR)/strchr.c    \
	$(LOCDIR)/strcmp.c    \
	$(LOCDIR)/strcoll.c   \
	$(LOCDIR)/strcpy.c    \
	$(LOCDIR)/strlcat.c   \
	$(LOCDIR)/strlcpy.c   \
	$(LOCDIR)/strlen.c    \
	$(LOCDIR)/strncat.c   \
	$(LOCDIR)/strncmp.c   \
	$(LOCDIR)/strncpy.c   \
	$(LOCDIR)/strnicmp.c  \
	$(LOCDIR)/strnlen.c   \
	$(LOCDIR)/strpbrk.c   \
	$(LOCDIR)/strrchr.c   \
	$(LOCDIR)/strspn.c    \
	$(LOCDIR)/strstr.c    \
	$(LOCDIR)/strtok.c    \
	$(LOCDIR)/strxfrm.c

LIBSTRING_CFLAGS += $(GLOBAL_CFLAGS) $(KERNEL_CFLAGS) $(INCLUDES)

# Include arch-specific
-include $(LOCDIR)/arch/$(ARCH)/Makefile.arch

$(LIBSTRING):
	$(Q)echo "Assembling [$(LIBSTRING)]..."
	$(Q)$(AR) r $(LIBSTRING) $(LIBSTRING_OBJ)
