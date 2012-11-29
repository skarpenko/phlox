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
