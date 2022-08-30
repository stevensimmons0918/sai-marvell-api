# New Makefile for Ros Over linux WM using MINGW64
#

VC_CROSS_COMPILE = x86_64-w64-mingw32-
CL = $(VC_CROSS_COMPILE)gcc
LINK = $(VC_CROSS_COMPILE)gcc
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
	external_phy/mtd/src \
    external_phy/alaska_c/src \
	src

GNU_DEBUG_CFLAGS = -g -O0
GNU_BASE_CFLAGS	= $(DEBUG_CFLAGS) -Wall -Wno-comment -Wno-pointer-sign -Wno-format
GNU_CFLAGS = $(GNU_BASE_CFLAGS) -Wno-pointer-sign -Wunused-variable
BASE_CFLAGS = -g -D"PHY_SIMULATION" -D"ASIC_SIMULATION"

override CFLAGS += $(BASE_CFLAGS)
override CFLAGS += \
 -I$(CPSS_FOLDER)/ \
 -I$(CPSS_FOLDER)/common/h/ \
 -I$(CPSS_FOLDER)/mainPpDrv/h/ \
 -I$(EZB_FOLDER)/pdl/h/ \
 -I$(EZB_FOLDER)/iDbgPdl/h/ \
 -Ih/ \
 -Iexternal_phy/mtd/src/ \
 -Iexternal_phy/alaska_c/src/

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

PHY_SRC :=
DEP :=
include $(patsubst %,%/$(MAKEFILE),$(MODULES))

GNU_CFLAGS += $(filter -D%,$(CFLAGS))
GNU_CFLAGS += $(filter -I%,$(CFLAGS))
#GNU_CFLAGS += $(patsubst /D"%",-D%,$(filter /D"%",$(CFLAGS)))
#GNU_CFLAGS += $(patsubst /I"%",-I%,$(filter /I"%",$(CFLAGS)))

VC_CFLAGS += $(filter -D%,$(CFLAGS))
VC_CFLAGS += $(filter -I%,$(CFLAGS))
override CFLAGS := $(VC_CFLAGS)

PHY_OBJ := $(patsubst %.c,$(OBJ_OUT_DIR)/%.$(OBJ_EXTENSION),$(PHY_SRC))
DEP += $(patsubst %.$(OBJ_EXTENSION),%.$(DEP_EXTENSION),$(PHY_OBJ))

###############################################################################
#
# %.obj - compile .c files to objects

$(OBJ_OUT_DIR)/%.$(DEP_EXTENSION): %.c
	$(call make_depend,$(GNU_CFLAGS),$(OBJ_OUT_DIR))

$(OBJ_OUT_DIR)/%.$(OBJ_EXTENSION):  %.c
	$(call make_depend,$(GNU_CFLAGS),$(OBJ_OUT_DIR))
	$(call compile,$(CFLAGS),$(OBJ_OUT_DIR))

###############################################################################
# targets
$(PHY_LIB): $(PHY_OBJ)
	$(call make_lib)

.PHONY: all
all: $(PHY_LIB)

lib: $(PHY_LIB)

clean:
	$(RM) $(OUT_DIR)

#######################################################

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif
print-%  : ; @echo $* = $($*)
