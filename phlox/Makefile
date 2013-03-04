#
# Main makefile
#

ifeq ($(MAKEFILE_LIST),)
$(warning This build system requires the MAKEFILE_LIST feature,)
$(warning which was added to GNU make in version 3.80.)
$(warning You are running $(MAKE_VERSION).)
$(error Required make feature missing)
endif


TARGETS   :=
FINAL_DEP :=


.PHONY: all
all: final


include mkconfig/tools.mk
include mkconfig/macro.mk
include mkconfig/make.config


include $(call include-makefiles,Makefile.mk)



final: $(FINAL_DEP)
	$(Q)echo "Boot image $(PHLOXZ) created."
	$(Q)echo "All done."





###ifneq "$(MAKECMDGOALS)" "clean"

#$(foreach TARGET,$(TARGETS),$(foreach DIR,$(sort $(dir $($(TARGET)_SRC))),$(call gen-c-rule,$(TARGET),$(BUILD_DIR),$(DIR))))
#$(foreach TARGET,$(TARGETS),$(foreach DIR,$(sort $(dir $($(TARGET)_SRC))),$(call gen-s-rule,$(TARGET),$(BUILD_DIR),$(DIR))))

$(foreach TARGET,$(TARGETS),$(foreach DIR,$(sort $(dir $($(TARGET)_SRC))),$(call gen-rules,$(TARGET),$(BUILD_DIR),$(DIR))))
$(foreach TARGET,$(TARGETS),$(call def-target,$(TARGET),$(BUILD_DIR)))
-include $(foreach TARGET,$(TARGETS),$(addprefix $(BUILD_DIR)/,$(call src-to-dep,$($(TARGET)_SRC))))

##endif


.PHONY: qemu
qemu: final
	qemu-system-i386 -fda $(PHLOXZ)

.PHONY: clean_test
clean_test:
	-rm -f $(foreach TARGET,$(TARGETS),$($(TARGET)_OBJ))
	-rm -f $(foreach TARGET,$(TARGETS),$(addprefix $(BUILD_DIR)/,$(call src-to-dep,$($(TARGET)_SRC))))


#clean:
#	rm -f $(ALL_OBJ) $(ALL_OTHER_INTERM)

#allclean:
#	rm -rf $(BUILD_DIR)

#mrproper:
#	rm -rf build/

#help:
#	@echo
#	@echo "Phlox Makefile options"
#	@echo
#	@echo "example: make <command> CPU=<cpu type> DEBUG=<yes/no>"
#	@echo
#	@echo "Commands:"
#	@echo "  help     - this help message;"
#	@echo "  final    - build Phlox boot image;"
#	@echo "  clean    - remove results of compilation;"
#	@echo "  allclean - remove target dir;"
#	@echo "  mrproper - clean up Phlox source tree;"
#	@echo "  bochs    - start Bochs virtual machine."
#	@echo
#	@echo "Additional options:"
#	@echo "  CPU=<cpu type>  - specifies CPU type to optimize for (default: i486);"
#	@echo "  DEBUG=yes|no    - turn on debug compilation (default: yes)."
#	@echo

# TODO: add more options help

#bochs: final
#	bochs -q 'boot:a' 'floppya: 1_44=$(PHLOXZ), status=inserted'
