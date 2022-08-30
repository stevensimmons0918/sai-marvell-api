################################################################################
# this file recursively included in a main makefile
#
# dir.mk file does everything that is specific to its directory and below,
# and recursively so:
# it can include dir.mk files in subdirectories
# these files do everything specific to their directories and below
#
# $D/ is relative path from the main Makefile to this directory
#
# all the variables you initialize, and all make targets with recipes, should be prepended with $D/
# (to disambiguate them in the main Makefile)
#
# $P/ is relative path to the "parent" - the directory of dir.mk that included you
################################################################################

################################################################################
# put this at the top of each dir.mk
################################################################################
$(TOP_MAGIC)


################################################################################
# all compiled files that go into a library
#
# only list here files for which .o objects are used to link with
# do not list files with main() that compile to executables
#
# they should be in this directory tree - in this directory or below
# if you have files not in this directory tree, write their own dir.mk
################################################################################
$D/LIB_EXAMPLES_SRCS := \
	$D/xpAppSignalHandler.c \
	$D/xpAppUtil.c \

$D/LIB_SRCS := \
	$($D/LIB_EXAMPLES_SRCS)

################################################################################
# all XP source files handled in this makefile fragment, needed for customer, that cannot be deduced from other source files
#
# do list all source control C/C++ sources written by XP, but
# do not list files that can be deduced from others - currently
# only included files can be deduced by a GCC option, so do not list included C/C++ files
#
# do not include any files in "black boxes" given by other vendors
#
# do list all C/C++ files that are generated automatically from other sources and are meant to be read by the customer
# so, shadow hardware files - yes
# but, SWIG .cxx wrapper - no
#
# if a generated file is listed, then do not list source files that are only needed to generate it
# but do list all the other non-C/C++ source control files
#
# all files listed should be in this directory tree - in this directory or below
# if you have files not in this directory tree, write their own dir.mk
################################################################################
$D/SRCS := $($D/LIB_SRCS)

$D/INCLUDES := \
	$(CPSS_INCLUDES) \
	-iquote $(REL_TOP)/xps/include \
	-iquote $(REL_TOP)/utils/state/include \
	-iquote $(REL_TOP)/utils/sal/include \
	-iquote $(REL_TOP)/demo/common/include \
	-iquote $(REL_TOP)/demo/common/py \
	-iquote $(REL_TOP)/demo/xpApp/include \
	-iquote $(REL_TOP)/demo/xpsApp/include \
	-iquote $(REL_TOP)/demo/config/include \
	-iquote $(REL_TOP)/demo/xpApp/l2/include \
	-iquote $(REL_TOP)/demo/xpApp/l3/include \
	-iquote $(REL_TOP)/demo/xpApp/tunnel/include \
	-iquote $(REL_TOP)/demo/xpsApp/l2/include \
	-iquote $(REL_TOP)/demo/xpsApp/l3/include \
	-iquote $(REL_TOP)/demo/xpsApp/tunnel/include \

ifdef MAC
	$D/INCLUDES += -I../lib
endif

$D/CXXFLAGS  := $(GCXXFLAGS) $(call PROD_VAR,CXXFLAGS) $($D/INCLUDES)

$D/CXX = $(CROSS_COMPILE)gcc  ${XP_SYSROOT}
$D/all: $($D/LIB)

################################################################################
# the lib in this directory has a specific explicit rule that overrides the general pattern rule for libraries
# put $$ in the prerequisite list when computing values of variables that are related to the target 
# or may be defined below us - $$ expands the variables with the "second pass"
################################################################################
$($D/LIB): $$($D/LIB_OBJS) $$(MKFS) | $$(@D)/.
	$(AR) $(ARFLAGS) $@ $(filter %.o,$^)


################################################################################
# put this immediately above bottom magic 
# includes dir.mk files in these subdirectories
################################################################################
$D/SUBDIRS := \
	\

$(call INCL_SUBDIRS, $($D/SUBDIRS))

################################################################################
# put this at the bottom of each dir.mk
################################################################################
$(BOTTOM_MAGIC)
