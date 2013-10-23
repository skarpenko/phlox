$(call def-target-vars,TEST_MAIN,test_main.elf)

TEST_MAIN_SRC +=               \
	$(LOCDIR)/test_main.c  \
	$(LOCDIR)/test0.c      \
	$(LOCDIR)/test1.c      \
	$(LOCDIR)/test2.c      \
	$(LOCDIR)/test3.c      \
	$(LOCDIR)/test4.c      \
	$(LOCDIR)/test5.c      \
	$(LOCDIR)/test6.c

TEST_MAIN_DEP = $(LIBPHLOX) $(LIBSTRING)

TEST_MAIN_CFLAGS += $(GLOBAL_CFLAGS) $(USER_CFLAGS) $(INCLUDES)

$(TEST_MAIN):
	$(Q)echo "Linking [$(TEST_MAIN)]"
	$(Q)$(LD) $(GLOBAL_LDFLAGS) -L$(LIBGCC_PATH) -L$(SERVICE_ARCH_PATH) -dN -script=$(SERVICE_LDSCRIPT) -o $(TEST_MAIN) $(TEST_MAIN_OBJ) $(LIBPHLOX) $(LIBGCC) $(LIBSTRING)
