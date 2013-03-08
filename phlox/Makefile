#
# Main makefile
#

ifeq ($(MAKEFILE_LIST),)
$(warning This build system requires the MAKEFILE_LIST feature,)
$(warning which was added to GNU make in version 3.80.)
$(warning You are running $(MAKE_VERSION).)
$(error Required make feature missing)
endif


# Targets list
TARGETS   :=
# Final target dependencies list
FINAL_DEP :=


.PHONY: all
all: final


# Include macro definitions and configuration
include mkconfig/tools.mk
include mkconfig/macro.mk
include mkconfig/make.config


# Print welcome and some info
$(info )
$(info *******************************************************************************)
$(info Phlox build system)
$(info *******************************************************************************)
$(info Build arguments:)
$(info - ARCH        = $(ARCH))
$(info - PLATFORM    = $(PLATFORM))
$(info - CPU         = $(CPU))
$(info - MACHINE     = $(MACHINE))
$(info - DEBUG       = $(DEBUG))
$(info - LIBGCC_PATH = $(LIBGCC_PATH))
$(info - BUILD_DIR   =  $(BUILD_DIR))
$(shell mkdir -p $(BUILD_DIR))
$(info *******************************************************************************)


# Include all top-level makefiles
include $(call include-makefiles,Makefile.mk)


.PHONY: final
final: $(FINAL_DEP)
	$(Q)echo "Boot image $(PHLOXZ) created."
	$(Q)echo "All done."


.PHONY: bochs
bochs: final
	$(Q)echo "Launching Bochs ..."
	$(Q)bochs -q 'boot:a' 'floppya: 1_44=$(PHLOXZ), status=inserted'


.PHONY: qemu
qemu: final
	$(Q)echo "Launching QEMU ..."
	$(Q)qemu-system-i386 -fda $(PHLOXZ)


.PHONY: clean
clean: $(foreach TARGET,$(TARGETS),$(TARGET)_clean)
	-rm -f $(foreach TARGET,$(TARGETS),$(addprefix $(BUILD_DIR)/,$(call src-to-obj,$($(TARGET)_SRC))))
	-rm -f $(foreach TARGET,$(TARGETS),$(addprefix $(BUILD_DIR)/,$(call src-to-dep,$($(TARGET)_SRC))))


.PHONY: allclean
allclean: $(foreach TARGET,$(TARGETS),$(TARGET)_allclean)
	-rm -rf $(BUILD_DIR)


.PHONY: mrproper
mrproper: $(foreach TARGET,$(TARGETS),$(TARGET)_mrproper)
	-rm -rf build/


.PHONY: help
help:
	@echo
	@echo "Phlox build system targets and options"
	@echo
	@echo "Example:"
	@echo "   make <target> CPU=<cpu type> DEBUG=<yes/no>"
	@echo
	@echo "Targets:"
	@echo "  help     - this help message;"
	@echo "  final    - build Phlox boot image (default);"
	@echo "  clean    - remove results of compilation;"
	@echo "  allclean - remove target dir;"
	@echo "  mrproper - clean up Phlox source tree;"
	@echo "  bochs    - start Bochs virtual machine;"
	@echo "  qemu     - start QEMU virtual machine."
	@echo
	@echo "Options:"
	@echo "  ARCH=<type>       - specifies target architecture (default: i386);"
	@echo "  PLATFORM=<type>   - specifies target platform (default: pc);"
	@echo "  CPU=<type>        - specifies CPU type to optimize for (default: i486);"
	@echo "  DEBUG=yes|no      - turn on debug compilation (default: yes);"
	@echo "  BUILD_ROOT=<path> - specifies path to build root directory;"
	@echo "  Q=<@| >           - if Q is empty build produces verbose output."
	@echo


# Define empty per target rules for clean goals
$(foreach TARGET,$(TARGETS),$(call def-phony-target,$(TARGET)_clean))
$(foreach TARGET,$(TARGETS),$(call def-phony-target,$(TARGET)_allclean))
$(foreach TARGET,$(TARGETS),$(call def-phony-target,$(TARGET)_mrproper))


# Generate generic compilation rules
$(foreach TARGET,$(TARGETS),$(foreach DIR,$(sort $(dir $($(TARGET)_SRC))),$(call gen-rules,$(TARGET),$(BUILD_DIR),$(DIR))))


# Generate target dependency rules
$(foreach TARGET,$(TARGETS),$(call def-target,$(TARGET),$(BUILD_DIR)))


# Include dependency files if not clean target passed to make
ifeq ($(call lists-intersect,clean allclean mrproper,$(MAKECMDGOALS)),)
-include $(foreach TARGET,$(TARGETS),$(addprefix $(BUILD_DIR)/,$(call src-to-dep,$($(TARGET)_SRC))))
endif
