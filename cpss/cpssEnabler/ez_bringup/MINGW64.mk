# New Makefile for iDbgPdl linux WM using MINGW64
#

VC_CROSS_COMPILE = x86_64-w64-mingw32-
CL = $(VC_CROSS_COMPILE)gcc
LINK = $(VC_CROSS_COMPILE)ld
LIB = $(VC_CROSS_COMPILE)ar

CC = $(CL)
LD = $(LINK)
AR = $(LIB)

GNU_CROSS_COMPILE = x86_64-w64-mingw32-
GNU_CC = $(GNU_CROSS_COMPILE)gcc
GNU_LD = $(GNU_CROSS_COMPILE)gcc
GNU_AR = $(GNU_CROSS_COMPILE)ar

ECHO = echo
MKDIR = mkdir -p
CP = cp
RM = rm -rf
MV = mv

# Name of makefile to include in each sub-module
MAKEFILE := Makefile

###############################################################################

MODULES := \
	$(PDL_DIR)/src \
	$(IDBG_PDL_DIR)/src

GNU_DEBUG_CFLAGS = -g -O0
GNU_BASE_CFLAGS	= $(DEBUG_CFLAGS) -Wall -Wno-comment -Wno-pointer-sign -Wno-format
GNU_CFLAGS = $(GNU_BASE_CFLAGS) -Wno-pointer-sign -Wunused-variable
BASE_CFLAGS = -g -D_CRT_SECURE_NO_WARNINGS -D_PDL_EXT_DRV_NOT_SUPPORT

override CFLAGS += $(BASE_CFLAGS)
override CFLAGS += -I$(MKFILE_DIR)/pdl/h -I$(MKFILE_DIR)/iDbgPdl/h

# Function compile(flags, output_folder)
#
# Compile a .c file into .o file using the defined toolchain, including dependecy .d file.
# Object and dependency files will be located together in output_folder,
# under the relative path to the source.
#
# Example: $(call compile $(CFLAGS),/local/objects)
# Source file: foo/bar/baz.c
# Targets: /local/objects/foo/bar/baz.o
#          /local/objects/foo/bar/baz.d

define make_depend
@# Create output folder
	@ $(MKDIR) $(dir $@)
@# print make_depend command
	@ echo MD $<
@# create dependency file
	@$(ECHO) $(GNU_CC) -MM -c $1 $< -o $2/$*.$(DEP_EXTENSION)
	@ $(GNU_CC) -MM -c $1 $< -o $2/$*.$(DEP_EXTENSION)
@# Duplicate it
	@ cp $2/$*.$(DEP_EXTENSION) $2/$*.tmp
@# Create empty rule per file in dependency rule to avoid "No rule to make target ..." Errors
	@ sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/d' -e 's/$$/ :/' < $2/$*.$(DEP_EXTENSION) >> $2/$*.tmp
@# Change object filename to include full path
	@ sed -e 's/$(subst .,\.,$(notdir $*.o))/$(subst /,\/,$2/$*.$(OBJ_EXTENSION))/'  < $2/$*.tmp > $2/$*.$(DEP_EXTENSION)
@# Remove temporary file
	@ $(RM) $2/$*.tmp
endef

define compile
@# Create output folder
	@ $(MKDIR) $(dir $@)
@# print compilation msg
	@ $(ECHO) CL $<
@# compile
	@ $(ECHO) $(CL) -c $1 $< -o $@
	@ $(CL) -c $1 $< -o $@
endef

define make_lib
	@ $(MKDIR) $(dir $@)
	@ $(ECHO) AR $@
	@ $(AR) -crs $@ $^
endef

define make_lib_from_obj_list
	@ $(MKDIR) $(dir $@)
	@ $(ECHO) AR $@
	@ $(AR) -crs $@ $1
endef

IDBG_PDL_SRC :=
PDL_SRC :=
DEP_PDL :=
DEP_IDBGPDL :=
include $(patsubst %,%/$(MAKEFILE),$(MODULES))

GNU_CFLAGS += $(filter -D%,$(CFLAGS))
GNU_CFLAGS += $(filter -I%,$(CFLAGS))
VC_CFLAGS += $(filter -D%,$(CFLAGS))
VC_CFLAGS += $(filter -I%,$(CFLAGS))
CFLAGS := $(VC_CFLAGS)

IDBG_PDL_OBJ := $(patsubst $(IDBG_PDL_DIR)%.c,$(IDBG_PDL_OBJ_OUT_DIR)%.$(OBJ_EXTENSION),$(IDBG_PDL_SRC))
PDL_OBJ := $(patsubst $(PDL_DIR)%.c,$(PDL_OBJ_OUT_DIR)%.$(OBJ_EXTENSION),$(PDL_SRC))

DEP_IDBGPDL += $(patsubst %.$(OBJ_EXTENSION),%.$(DEP_EXTENSION),$(IDBG_PDL_OBJ))
DEP_PDL += $(patsubst %.$(OBJ_EXTENSION),%.$(DEP_EXTENSION),$(PDL_OBJ))

###############################################################################
#
# %.obj - compile .c files to objects

$(IDBG_PDL_OBJ_OUT_DIR)/%.$(DEP_EXTENSION):  $(IDBG_PDL_DIR)/%.c
	$(call make_depend,$(GNU_CFLAGS),$(IDBG_PDL_OBJ_OUT_DIR))

$(IDBG_PDL_OBJ_OUT_DIR)/%.$(OBJ_EXTENSION):  $(IDBG_PDL_DIR)/%.c
	#$(call make_depend,$(GNU_CFLAGS),$(IDBG_PDL_OBJ_OUT_DIR))
	$(call compile,$(CFLAGS),$(IDBG_PDL_OBJ_OUT_DIR))

$(PDL_OBJ_OUT_DIR)/%.$(DEP_EXTENSION):  $(PDL_DIR)/%.c
	$(call make_depend,$(GNU_CFLAGS),$(PDL_OBJ_OUT_DIR))

$(PDL_OBJ_OUT_DIR)/%.$(OBJ_EXTENSION):  $(PDL_DIR)/%.c
	#$(call make_depend,$(GNU_CFLAGS),$(PDL_OBJ_OUT_DIR))
	$(call compile,$(CFLAGS),$(PDL_OBJ_OUT_DIR))

###############################################################################
# targets

.PHONY: all clean pdl_clean idbgpdl_clean pdl_lib idbg_pdl_lib
all: $(PDL_LIB) $(IDBG_PDL_LIB)

$(IDBG_PDL_LIB): $(IDBG_PDL_OBJ)
	$(call make_lib)

$(PDL_LIB): $(PDL_OBJ)
	$(call make_lib)

pdl_lib: $(PDL_LIB)

idbg_pdl_lib: $(IDBG_PDL_LIB)

lib: pdl_lib idbg_pdl_lib

pdl_clean:
	$(RM) $(PDL_OUT_DIR)

idbg_pdl_clean:
	$(RM) $(IDBG_PDL_OUT_DIR)

clean: pdl_clean idbg_pdl_clean

print-%  : ; @echo $* = $($*)

#######################################################
ifeq ($(filter-out pdl_clean pdl_lib, $(MAKECMDGOALS)),)
ifneq ($(filter-out $(MAKECMDGOALS), pdl_clean),)
-include $(DEP_PDL)
endif
endif
ifeq ($(filter-out idbg_pdl_clean idbg_pdl_lib, $(MAKECMDGOALS)),)
ifneq ($(filter-out $(MAKECMDGOALS), idbg_pdl_clean),)
-include $(DEP_IDBGPDL)
endif
endif
