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
# You must never use explicit absolute paths beginning with �/� when referring to any 
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
# include paths options for compiler
#
# $($P/INCLUDES) is the paths from the parent dir.mk, put them only if they are needed
#
# you can use relative paths from the top (xdk) for directories not under $D
# 
# Here we have used -I for the xai include path because they have inlcuded the
#
# saitypes.h as #include <saitypes.h> and for this the -iquode doesnt work.
################################################################################
$D/INCLUDES := \
	-iquote $(REL_TOP)/xps/include \
	-iquote $(REL_TOP)/demo/config/include \
	-iquote $(REL_TOP)/demo/common/py \
	-iquote $(REL_TOP)/$(SAI_SRC)/xpSai/include \
	-iquote $(REL_TOP)/$(SAI_SRC)/xpSaiApp/tunnel/include \
	-iquote $(REL_TOP)/utils/state/include \
	-iquote $(REL_TOP)/utils/allocator/include \
	-iquote $(REL_TOP)/utils/sal/include \
	-I $(REL_TOP)/$(SAI_SRC)/xpSai/sai \
	-I $(REL_TOP)/$(SAI_SRC)/xpSai/sai/inc \
	-iquote $(REL_TOP)/$(SAI_SRC)/xpSai/util/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalDeviceMgr/include \
	-iquote $(REL_TOP)/cpssHal/cpssHalInit/include/ \
        -iquote $(REL_TOP)/cpssHal/cpssHalQos/include/ \
	-iquote $(REL_TOP)/cpssHal/cpssHalUtil/include/ \
	-iquote $(REL_TOP)/cpssHal/cpssHalCounter/include/ \
	-iquote $(REL_TOP)/cpss/ \
	$(CPSS_INCLUDES) \
	-I xpSai/sai/inc

###############################################################################
# MAC is defined only on MAC
#
# this is how you can do custom MAC stuff
###############################################################################
ifdef MAC
	$D/INCLUDES += -I../lib
endif


###############################################################################
# specify compiler options for this directory - mandatory if you have files compiled here
# 
# $(GCXXFLAGS) are the default options defined in variables.mk
###############################################################################
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
$D/LIB_XDK_XPSAI_SRCS := \
     $D/xpSaiSwitch.c \
     $D/xpSaiPort.c \
     $D/xpSaiPortCfgManager.c \
     $D/xpSaiFdb.c \
     $D/xpSaiVlan.c \
     $D/xpSaiVrf.c \
     $D/xpSaiVirtualRouter.c \
     $D/xpSaiRoute.c \
     $D/xpSaiNextHop.c \
     $D/xpSaiNextHopGroup.c \
     $D/xpSaiRouterInterface.c \
     $D/xpSaiHostInterface.c \
     $D/xpSaiHostIfFd.c \
     $D/xpSaiHostIfNetdev.c \
     $D/xpSaiHostIfGenetlink.c \
     $D/xpSaiCopp.c \
     $D/xpSaiNeighbor.c \
     $D/xpSaiAcl.c \
     $D/xpSaiAclMapper.c \
     $D/xpSaiStp.c \
     $D/xpSaiLag.c \
     $D/xpSaiLog.c \
     $D/xpSaiSamplePacket.c \
     $D/xpSaiMirror.c \
     $D/xpSaiCounter.c \
     $D/xpSai.c \
     $D/util/xpSaiDev.c \
     $D/util/xpSaiUtil.c \
     $D/xpSaiPolicer.c \
     $D/xpSaiWred.c \
     $D/xpSaiQosMap.c \
     $D/xpSaiQueue.c \
     $D/xpSaiScheduler.c \
     $D/xpSaiSchedulerGroup.c \
     $D/xpSaiBuffer.c \
     $D/xpSaiHash.c \
     $D/xpSaiUdf.c \
     $D/xpSaiBridge.c \
     $D/xpSaiObject.c \
     $D/xpSaiMcastFdb.c \
     $D/xpSaiL2Mc.c \
     $D/xpSaiL2McGroup.c \
     $D/xpSaiTunnel.c \
     $D/xpSaiIpmc.c \
     $D/xpSaiIpmcGroup.c \
     $D/xpSaiRpfGroup.c\
     $D/xpSaiProfileMng.c

$D/LIB_SRCS := \
    $($D/LIB_XDK_XPSAI_SRCS) \

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
$D/LIB := $(OBJSDIR)/$D/libXdkXpSai.a


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
