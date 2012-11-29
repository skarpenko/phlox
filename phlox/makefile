ifeq ($(MAKEFILE_LIST),)
$(warning This build system requires the MAKEFILE_LIST feature,)
$(warning which was added to GNU make in version 3.80.)
$(warning You are running $(MAKE_VERSION).)
$(error Required make feature missing)
endif

# used by included makefiles
ALL_OBJ          :=
ALL_OTHER_INTERM :=

all: final


include targets.mk
include make.config
include macros.mk
include tools/makefile
include lib/makefile
include kernel/makefile
include boot/makefile

final: $(PHLOXZ)
	@echo Boot image $(PHLOXZ) created.
	@echo All done.

clean:
	rm -f $(ALL_OBJ) $(ALL_OTHER_INTERM)

allclean:
	rm -rf $(BUILD_DIR)

mrproper:
	rm -rf build/

help:
	@echo
	@echo "Phlox Makefile options"
	@echo
	@echo "example: make <command> CPU=<cpu type>"
	@echo
	@echo "Commands:"
	@echo "  help     - this help message;"
	@echo "  final    - build Phlox boot image;"
	@echo "  clean    - remove results of compilation;"
	@echo "  allclean - remove target dir;"
	@echo "  mrproper - clean up Phlox source tree;"
	@echo "  bochs    - start Bochs virtual machine."
	@echo
	@echo "Additional options:"
	@echo "  CPU=<cpu type>  - specifies CPU type to optimize for;"
	@echo "  DEBUG=yes       - turn on debug compilation."
	@echo

bochs: final
	bochs -q 'boot:a' 'floppya: 1_44=$(PHLOXZ), status=inserted'
