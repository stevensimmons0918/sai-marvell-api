################################################################################
# This file recursively included in a main makefile.
#
# dir.mk file does everything that is specific to its directory and below,
# and recursively so:
# it can include dir.mk files in subdirectories
# these files do everything specific to their directories and below.
#
# $D/ is relative path from the main Makefile to this directory.
#
# You must never use explicit absolute paths beginning with <93>/<94> when referring to any
# source code, make variables, make real and phony targets, and any build artefacts.  Always use relative paths starting from the top.  

# Always use $D/ when referring to your directory and the directory tree below you.  
# Otherwise, use relative paths from the main software directory (xdk)

# You can use absolute paths referring to any tools or places outside the source code and build tree.  
# But in this case it is best to use the variables defined in variables.mk.

#
# All the variables you initialize (with := or = or ?=) and use in recipes, and 
# all make targets with recipes, 
# should be prepended with $D/
# (to disambiguate them in the main Makefile).
#
# $P/ is relative path to the "parent" - the directory of makefile that included you.
#
# You can use absolute paths referring to any tools or places outside the source and build tree.  
################################################################################

################################################################################
# MANDATORY
# 
# put this at the top of each dir.mk
################################################################################
$(TOP_MAGIC)

################################################################################
# Local Variables
# 
################################################################################
################################################################################
# include paths options for compiler
#
# $($P/INCLUDES) is the paths from the parent dir.mk, put them only if they are needed
#
# you can use relative paths from the top (xdk) for directories not under $D
################################################################################
$D/INCLUDES := \
	$(CPSS_INCLUDES) \
	-iquote $(REL_TOP)/xps/include \
	-iquote $(REL_TOP)/utils/state/include \
	-iquote $(REL_TOP)/utils/sal/include \
	-iquote $(REL_TOP)/utils/allocator/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalFdb/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalCopp/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalCtrlPkt/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalLinkMgr/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalDeviceMgr/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalUtil/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalCascadeMgr/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalHostIntfMgr/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalVlan/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalLag/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalStp/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalNhgrp/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalL3/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalQos/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalAcl/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalTunnel/include \
	-I$(REL_TOP)/cpssHal/cpssHalInit/include \
	-I$(REL_TOP)/cpssHal/cpssHalPlatform/include \
	-I$(REL_TOP)/cpssHal/cpssHalUtil/include/ \
	-I$(REL_TOP)/cpssHal/cpssHalShell/include/ \
	-I$(REL_TOP)/cpssHal/cpssHalSim/include \
	-I$(REL_TOP)/cpssHal/cpssHalExt/include \
	-I$(REL_TOP)/cpssHal/cpssHalDeviceMgr/include \
	-I$(REL_TOP)/cpssHal/cpssHalTunnel/include \
	-I$(REL_TOP)/cpssHal/cpssHalMirror/include \
	-I$(REL_TOP)/cpssHal/cpssHalCounter/include \
	-I$(REL_TOP)/cpssHal/cpssHalTcam/include \
	-I$(REL_TOP)/cpssHal/cpssHalMulticast/include \
	-I$(REL_TOP)/cpssHal/cpssHalPha/include \
	$(CPSS_INCLUDES)

################################################################################
# MAC is defined only on MAC
#
# this is how you can do custom MAC stuff
################################################################################
ifdef MAC
	$D/INCLUDES += -I../lib
endif

################################################################################
# specify compiler options for this directory - mandatory if you have files compiled here
# 
# $(GCXXFLAGS) are the default options defined in variables.mk
################################################################################
$D/CXXFLAGS  := $(GCXXFLAGS) $(call PROD_VAR,CXXFLAGS) $($D/INCLUDES)


################################################################################
# all compiled files that go into a library
#
# only list here files for which .o objects are used to link with
# do not list files with main() that compile to executables
#
# they should be in this directory tree - in this directory or below
# if you have files not in this directory tree, write their own dir.mk
################################################################################

$D/LIB_XDK_XPS_SRCS := \
	$D/xpsXpImports.c \
	$D/xpsMulticast.c \
	$D/xpsLag.c \
	$D/xpsMirror.c \
	$D/xpsNat.c \
	$D/xpsFdb.c \
	$D/xpsFdbMgr.c \
	$D/xpsL3.c \
	$D/xps8021Br.c \
	$D/xpsNhGrp.c \
	$D/xpsAcm.c \
	$D/xpsQos.c \
	$D/xpsPacketTrakker.c \
	$D/xpsCopp.c \
	$D/xpsInt.c \
	$D/xpsGlobalSwitchControl.c \
	$D/xpsSflow.c \
	$D/xpsPort.c \
	$D/xpsPacketDrv.c \
	$D/xpsInterface.c \
	$D/xpsEgressFilter.c \
	$D/xpsVlan.c \
	$D/xpsStp.c \
	$D/xpsTunnel.c \
	$D/xpsInit.c \
	$D/xpsMpls.c \
	$D/xpsLink.c \
	$D/xpsMac.c \
	$D/xpsMtuProfile.c \
	$D/xpsSerdes.c \
	$D/xpsAcl.c \
	$D/xpsUtil.c 
$D/LIB_XDK_XPS_SRCS += \
	$D/xpsPolicer.c \
	$D/xpsVxlan.c \
	$D/xpsGeneve.c \
	$D/xpsNvgre.c \
	$D/xpsIpGre.c \
	$D/xpsVpnGre.c \
	$D/xpsIpinIp.c \
	$D/xpsErspanGre.c \
	$D/xpsAging.c \
	$D/xpsOpenflow.c \
	$D/xpsScope.c \
	$D/xpsPtp.c \
	$D/xpsPbb.c \
	$D/xpsLock.c \
	$D/xpsSr.c \
	$D/xpsVsi.c \
	$D/xpsAc.c \
	$D/xpsAllocator.c	


$D/LIB_SRCS := \
    $($D/LIB_XDK_XPS_SRCS) \
    $($D/LIB_XPS_STATE)
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
$D/SRCS := $($D/LIB_SRCS) \


################################################################################
# library archived here from the sources above
#
# should be in directory $(OBJSDIR)/$D
################################################################################
$D/LIB := $(OBJSDIR)/$D/libXdkXps.a


################################################################################
# list all "default" targets in this dir
# (if there are any)
################################################################################
$D/all: $($D/LIB) $($D/TEST)

################################################################################
# rules we need to add in this dir.mk
#
# the value of $D/LIB_OBJS is the objects from $($D/LIB_SRCS) and it is computed automatically later, and you have to evaluate it
# with double $ as:
# $$($D/LIB_OBJS)
#
# you also want to depend on all the makefiles that may affect the recipe for your rule
# the makefiles parsed so far are $(MKFS)
# all the makefiles parsed are $$(MKFS)
#
# $$(@D)/. is the directory of the target, which needs to be "updated" (created) before the target can be created
# we recognize a directory with the trailing /. and it is evaluated during second parsing pass (after the target is evaluated
# in the first), hence $$
#
# notice we obtain the recipes (as much as possible) from the target and prerequisites
# this is good software practice - define each thing in one place only
#
# if you need to use the current value of a variable that changes values, in a recipe, 
# you have to capture the current value in a target-specific variable, like this for the variable D
#
# target: D := $D
#
# now $D evaluated inside the recipe for "target" will be the "current" $D
################################################################################

################################################################################
# custom rule for library
# 
# library is built automatically from $$($D/LIB_OBJS) and using $(ARFLAGS)
# so specify it here only if you need something else
#
# this custom rule example adds objects from subdirectory subdir
################################################################################
$($D/LIB): $$($D/LIB_OBJS) $$(MKFS) | $$(@D)/.
	$(RM) -f $@
	$(AR) $(ARFLAGS) $@ $(filter %.o,$^)


################################################################################
# custom clean for this dir 
# there is an automatic clean of objects, libs and filelists,
# so specify here only if you need a different one
################################################################################
$D/clean : 
	@$(ECHO) Cleaning up $D ...	
	$(RM) -f $($D/TEST)
	$(RM) -f $(OBJSDIR)/$D/*.a $(OBJSDIR)/$D/*.o
	$(RM) -f $D/filelist.* $D/*_filelist.*



################################################################################
# put this immediately above bottom magic 
# includes dir.mk files in these subdirectories
################################################################################
$D/SUBDIRS := \

$(call INCL_SUBDIRS, $($D/SUBDIRS))


################################################################################
# MANDATORY
#
# put this at the bottom of each dir.mk
################################################################################
$(BOTTOM_MAGIC)
