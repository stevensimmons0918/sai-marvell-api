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
# w
# include paths options for compiler
#
# $($P/INCLUDES) is the paths from the parent dir.mk, put them only if they are needed
#
# you can use relative paths from the top (xdk) for directories not under $D
################################################################################
$D/INCLUDES := \
	-iquote $(REL_TOP)/cpssHal/cpssHalLinkMgr/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalUtil/include	\
	-iquote $(REL_TOP)/cpssHal/cpssHalPlatform/include  \
	-iquote $(REL_TOP)/cpssHal/cpssHalDeviceMgr/include	\
	-iquote $(REL_TOP)/cpssHal/cpssHalCascadeMgr/include	\
	-iquote $(REL_TOP)/cpssHal/cpssHalFdb/include	\
	-iquote $(REL_TOP)/cpssHal/cpssHalCopp/include	\
	-iquote $(REL_TOP)/cpssHal/cpssHalCtrlPkt/include	\
	-iquote $(REL_TOP)/cpssHal/cpssHalHostIntfMgr/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalVlan/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalLag/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalStp/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalNhgrp/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalL3/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalQos/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalAcl/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalTunnel/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalMirror/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalCounter/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalTcam/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalMulticast/include \
	-iquote $(REL_TOP)/xps/include  \
	-iquote $(REL_TOP)/utils/state/include \
	-iquote $(REL_TOP)/utils/sal/include \
	$(CPSS_INCLUDES)\
	-I$(REL_TOP)/cpssHal/cpssHalInit/include/ \
	-I$(REL_TOP)/cpssHal/cpssHalInit/include/polarity_configs \
	-I$(REL_TOP)/cpssHal/cpssHalInit/include/port_profiles \
	-I$(REL_TOP)/cpssHal/cpssHalInit/include/port_profiles/falcon_6_4_port_profile/ \
	-I$(REL_TOP)/cpssHal/cpssHalInit/include/port_profiles/falcon_12_8_port_profile/ \
	-I$(REL_TOP)/cpssHal/cpssHalInit/include/port_profiles/falcon_3_2_port_profile/ \
	-I$(REL_TOP)/cpssHal/cpssHalInit/include/port_profiles/falcon_2_port_profile/ \
	-I$(REL_TOP)/cpssHal/cpssHalInit/include/port_profiles/ac5x_port_profile/ \
	-I$(REL_TOP)/cpssHal/cpssHalUtil/include/ \
	-I$(REL_TOP)/cpssHal/cpssHalExt/include/ \
	-I$(REL_TOP)/cpssHal/cpssHalShell/include/ \
	-I$(REL_TOP)/cpssHal/cpssHalPha/include/ \
	-I$(REL_TOP)/cpssHal/cpssHalSim/include/  


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

GCXXFLAGS += -DLINUX -DUNIX
ifeq ($(TARGET),aarch64)
    GCXXFLAGS += -DINCLUDE_MPD
endif

$D/LIB_XDK_XPS_SRCS := \
	$(REL_TOP)/cpssHal/cpssHalLinkMgr/cpssHalMac.c \
	$(REL_TOP)/cpssHal/cpssHalLinkMgr/cpssHalPort.c \
	$(REL_TOP)/cpssHal/cpssHalLinkMgr/cpssHalPortMgr.c \
	$(REL_TOP)/cpssHal/cpssHalLinkMgr/cpssHalPortNoMgr.c \
	$(REL_TOP)/cpssHal/cpssHalPlatform/cpssHalPlatform.c \
	$(REL_TOP)/cpssHal/cpssHalPlatform/cpssHalSysKern.c \
	$(REL_TOP)/cpssHal/cpssHalLinkMgr/cpssHalLed.c \
	$(REL_TOP)/cpssHal/cpssHalLinkMgr/cpssHalPhy.c \
	$(REL_TOP)/cpssHal/cpssHalUtil/cpssHalUtil.c	\
	$(REL_TOP)/cpssHal/cpssHalUtil/cpssHalDumpUtil.c\
	$(REL_TOP)/cpssHal/cpssHalDeviceMgr/cpssHalDevice.c 	\
	$(REL_TOP)/cpssHal/cpssHalExt/cpssHalExt.c \
	$(REL_TOP)/cpssHal/cpssHalInit/cpssHalDev.c \
	$(REL_TOP)/cpssHal/cpssHalInit/cpssHalConfig.c \
	$(REL_TOP)/cpssHal/cpssHalInit/cpssHalInit.c \
	$(REL_TOP)/cpssHal/cpssHalInit/cpssHalSys.c \
	$(REL_TOP)/cpssHal/cpssHalInit/cpssHalPhyConfig.c \
	$(REL_TOP)/cpssHal/cpssHalShell/cpssHalShell.c \
	$(REL_TOP)/cpssHal/cpssHalUtil/cpssHalDumpUtil.c \
	$(REL_TOP)/cpssHal/cpssHalUtil/cpssHalUtilOsNetworkStack.c \
	$(REL_TOP)/cpssHal/cpssHalUtil/cpssHalOsNetworkStackLinux.c \
	$(REL_TOP)/cpssHal/cpssHalFdb/cpssHalFdb.c \
	$(REL_TOP)/cpssHal/cpssHalFdb/cpssHalFdbMgr.c \
	$(REL_TOP)/cpssHal/cpssHalCopp/cpssHalCopp.c \
	$(REL_TOP)/cpssHal/cpssHalCtrlPkt/cpssHalCtrlPkt.c \
	$(REL_TOP)/cpssHal/cpssHalHostIntfMgr/cpssHalHostIf.c \
	$(REL_TOP)/cpssHal/cpssHalVlan/cpssHalVlan.c \
	$(REL_TOP)/cpssHal/cpssHalLag/cpssHalLag.c \
	$(REL_TOP)/cpssHal/cpssHalCascadeMgr/cpssHalCascadeMgr.c	\
	$(REL_TOP)/cpssHal/cpssHalStp/cpssHalStp.c	\
	$(REL_TOP)/cpssHal/cpssHalNhgrp/cpssHalNhgrp.c	\
	$(REL_TOP)/cpssHal/cpssHalL3/cpssHalL3.c	\
	$(REL_TOP)/cpssHal/cpssHalQos/cpssHalQos.c	\
	$(REL_TOP)/cpssHal/cpssHalAcl/cpssHalAcl.c	\
	$(REL_TOP)/cpssHal/cpssHalTunnel/cpssHalTunnel.c \
	$(REL_TOP)/cpssHal/cpssHalCounter/cpssHalCounter.c \
	$(REL_TOP)/cpssHal/cpssHalTcam/cpssHalTcam.c \
	$(REL_TOP)/cpssHal/cpssHalMulticast/cpssHalMulticast.c \
	$(REL_TOP)/cpssHal/cpssHalPha/cpssHalPha.c \
	$(REL_TOP)/cpssHal/cpssHalMirror/cpssHalMirror.c



ifeq ("$(ARCH)", "x86")
$D/LIB_XDK_XPS_SRCS += \
	$(REL_TOP)/cpssHal/cpssHalSim/cpssHalSim.c 
endif


$D/LIB_SRCS := \
    $($D/LIB_XDK_XPS_SRCS)

$D/SRCS := $($D/LIB_SRCS) \


$D/LIB := $(OBJSDIR)/$D/libXdkXps.a

$($D/LIB): $$($D/LIB_OBJS) $$(MKFS) | $$(@D)/. 
	$(RM) -f $@
	$(AR) $(ARFLAGS) $@ $(filter %.o,$^)

## Workaround to disable Werror warnings for 
## aarch64 (arm 64 bit) for -wundef warnings
ifdef DISABLE_WARNING
ifeq ($(TARGET),aarch64)
GCXXFLAGS := $(filter-out -Werror, $(GCXXFLAGS)) 
CPPFLAGS := $(filter-out -Werror, $(CPPFLAGS))
endif 
ifeq ("$(GCCVERSION_WERROR)","1")
GCXXFLAGS := $(filter-out -Werror, $(GCXXFLAGS)) 
CPPFLAGS := $(filter-out -Werror, $(CPPFLAGS))
endif 
endif
 
$D/CXXFLAGS  := $(GCXXFLAGS) $(call PROD_VAR,CXXFLAGS) $($D/INCLUDES)

$D/CXX = $(CROSS_COMPILE)gcc  ${XP_SYSROOT}

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
