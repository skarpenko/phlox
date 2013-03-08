##
## Macro definitions
##


# Used to create a build directory in build rules.
CREATE_DIR_RULE = if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi

# gets the local directory of the makefile. Requires gmake 3.80+
LOCDIR = $(patsubst %/,%,$(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))))

# Define target related variables if was not defined before.
# $(call def-target-vars, target_name, target_filename)
define def-target-vars
$(eval \
    $(if $(filter $(origin $1_TARGET),undefined),$1_TARGET:=$$(BUILD_DIR)/$$(LOCDIR)/$2,)
    $(if $(filter $(origin $1_SRC),undefined),$1_SRC:=,)
    $(if $(filter $(origin $1_DEP),undefined),$1_DEP=,)
    $(if $(filter $(origin $1),undefined),TARGETS+=$1,)
    $(if $(filter $(origin $1),undefined),$1=$$($1_TARGET),)
)
endef

# Generate dependency files.
# $(call make-depend, src_file, obj_file, dep_file, flags)
define make-depend
    $(MAKEDEPEND) \
        -MM       \
        -MF $3    \
        -MP       \
        -MT $2    \
        $4        \
        $1
endef

# Generate C-rule.
# $(call gen-c-rule, target_name, build_dir, src_dir)
define gen-c-rule
$(eval \
$$(subst //,/,$2/$3/%.o): $$(subst //,/,$3/%.c)
	$$(Q)echo "Compiling [$$<]"
	$$(Q)$$(CREATE_DIR_RULE)
	$$(Q)$$(call make-depend,$$<,$$@,$$(subst .o,.d,$$@),$$($1_CFLAGS))
	$$(Q)$$(CC) $$($1_CFLAGS) -c $$< -o $$@
)
endef

# Generate asmembly-rule.
# $(call gen-s-rule, target_name, build_dir, src_dir)
define gen-s-rule
$(eval \
$$(subst //,/,$2/$3/%.o): $$(subst //,/,$3/%.S)
	$$(Q)echo "Compiling [$$<]"
	$$(Q)$$(CREATE_DIR_RULE)
	$$(Q)$$(call make-depend,$$<,$$@,$$(subst .o,.d,$$@),$$($1_ASFLAGS))
	$$(Q)$$(CC) $$($1_ASFLAGS) -c $$< -o $$@
)
endef

# Generate rules.
# $(call gen-rules, target_name, build_dir, src_dir)
define gen-rules
$(eval \
$(call gen-c-rule,$1,$2,$3) \
$(call gen-s-rule,$1,$2,$3)
)
endef

# Generate C-rule explicitly.
# $(call gen-explicit-c-rule, target_name, build_dir, src_dir, src_files_list)
define gen-explicit-c-rule
$(foreach src,$(filter %.c,$4), \
$(eval \
$$(subst //,/,$2/$3/$$(subst .c,.o,$(src))): $$(subst //,/,$3/$(src))
	$$(Q)echo "Compiling [$$<]"
	$$(Q)$$(CREATE_DIR_RULE)
	$$(Q)$$(call make-depend,$$<,$$@,$$(subst .o,.d,$$@),$$($1_CFLAGS))
	$$(Q)$$(CC) $$($1_CFLAGS) -c $$< -o $$@
))
endef

# Generate assemply-rule explicitly.
# $(call gen-explicit-s-rule, target_name, build_dir, src_dir, src_files_list)
define gen-explicit-s-rule
$(foreach src,$(filter %.S,$4), \
$(eval \
$$(subst //,/,$2/$3/$$(subst .S,.o,$(src))): $$(subst //,/,$3/$(src))
	$$(Q)echo "Compiling [$$<]"
	$$(Q)$$(CREATE_DIR_RULE)
	$$(Q)$$(call make-depend,$$<,$$@,$$(subst .o,.d,$$@),$$($1_ASFLAGS))
	$$(Q)$$(CC) $$($1_ASFLAGS) -c $$< -o $$@
))
endef

# Generate rules explicitly.
# $(call gen-explicit-rules, target_name, build_dir, src_dir, src_files_list)
define gen-explicit-rules
$(eval \
$(call gen-explicit-c-rule,$1,$2,$3,$4) \
$(call gen-explicit-s-rule,$1,$2,$3,$4)
)
endef

# Get list of object files from list of source files.
# $(call src-to-obj, src_filelist)
src-to-obj =                        \
    $(subst .c,.o,$(filter %.c,$1)) \
    $(subst .S,.o,$(filter %.S,$1))

# Get list of dependency files from list of source files.
# $(call src-to-dep, src_filelist)
src-to-dep =                        \
    $(subst .c,.d,$(filter %.c,$1)) \
    $(subst .S,.d,$(filter %.S,$1))

# Define target rule.
# $(call def-target, target_name, build_dir)
define def-target
$(eval \
    $1_OBJ := $(addprefix $2/,$(call src-to-obj,$($1_SRC)))
    $($1): $($1_DEP) $(addprefix $2/,$(call src-to-obj,$($1_SRC)))
)
endef

# Build list of files for include into makefile.
# include $(call include-makefiles, makefile_name, search_dir_optional)
define include-makefiles
$(addsuffix /$1,$(subst /$1,,$(shell find $(if $2,$2,.) -name $1)))
endef

# Returns 1 if two lists intersects otherwise empty string
# $(call lists-intersect, list1, list2)
lists-intersect = $(sort $(foreach ITM,$1,$(if $(filter $(ITM),$2),1,)))

# Defines PHONY target
# $(call def-phony-target, target_name)
define def-phony-target
$(eval \
    .PHONY: $1
    $1:
)
endef
