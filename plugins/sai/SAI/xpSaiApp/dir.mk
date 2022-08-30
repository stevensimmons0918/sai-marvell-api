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

$D/INCLUDES := \
	$(CPSS_INCLUDES) \
	-iquote $(REL_TOP)/$(SAI_SRC)/xpSaiApp/include \
	-iquote $(REL_TOP)/$(SAI_SRC)/xpSaiApp/l2/include \
	-iquote $(REL_TOP)/$(SAI_SRC)/xpSaiApp/l3/include \
	-iquote $(REL_TOP)/$(SAI_SRC)/xpSaiApp/tunnel/include \
	-iquote $(REL_TOP)/$(SAI_SRC)/xpSaiApp/sai/include \
	-iquote $(REL_TOP)/$(SAI_SRC)/xpSaiApp/qos/include \
	-iquote $(REL_TOP)/utils/state/include \
	-iquote $(REL_TOP)/utils/allocator/include \
	-iquote $(REL_TOP)/utils/sal/include \
	-iquote $(REL_TOP)/demo/common/include \
	-iquote $(REL_TOP)/demo/config/include \
	-iquote $(REL_TOP)/demo/common/py \
	-iquote $(REL_TOP)/$(SAI_SRC)/xpSai/include \
	-iquote $(REL_TOP)/$(SAI_SRC)/xpSai/util/include \
	-iquote $(REL_TOP)/xps/include \
	-I $(REL_TOP)/$(SAI_SRC)/xpSai/sai \
	-I $(REL_TOP)/$(SAI_SRC)/xpSai/sai/inc \
	-I $(REL_TOP)/cpssHalInit/include \
	-I $(REL_TOP)/cpssHal/cpssHalUtil/include/ \
	-I $(REL_TOP)/cpssHal/cpssHalDeviceMgr/include \
	-I $(REL_TOP)/cpssHal/cpssHalInit/include/ \
	-I $(REL_TOP)/cpssHal/cpssHalShell/include \
	-I $(REL_TOP)/cpssHalSim/include \

ifdef MAC
	$D/INCLUDES += -I../lib
endif

$D/CXXFLAGS  := $(GCXXFLAGS) $(call PROD_VAR,CXXFLAGS) -DABSOLUTE_PATH='"$(ABSOLUTE_DIR_PATH)"' -DCONFIG_PATH='$(CONFIG_DIR_PATH)' $($D/INCLUDES)

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
$D/LIB_SRCS := \
    $D/xpSaiApp.c \
    $D/l2/xpSaiAppL2.c \
    $D/l3/xpSaiAppL3.c \
    $D/xpSaiAppL3Uc.c \
    $D/tunnel/xpSaiAppTnl.c \
    $D/tunnel/xpSaiAppMpls.c \
    $D/sai/xpSaiAppSaiUtils.c \
    $D/qos/xpSaiAppQos.c \

$D/SRCS     := \
	$($D/LIB_SRCS) \
	$D/main.c


#################################################################################
# libraries needed for our executable, in order
################################################################################
LIBS_NEEDED_IN_ORDER :=
LIBS_NEEDED_IN_ORDER_UNITTEST :=

LIBS_NEEDED_IN_ORDER += \
	$(DIST)/libApp.a \
	$(DIST)/libXpPy.a \
	$(DIST)/libsaiShell.a \
	$(DIST)/libsaiLuaWrapper.a \
	$(DIST)/libXpSaiAdapter.a 
LIBS_NEEDED_IN_ORDER_UNITTEST += \
    $(LIBS_NEEDED_IN_ORDER_UNITTEST_WM) \
	$(DIST)/libXdk.a \

CPPS_LIBS_IN_ORDER := $(CPSS_LIBS)
CPPS_LIBS_IN_ORDER += $(DIST)/$(CPSS_ENABLER_LIB)
CPPS_LIBS_IN_ORDER += $(DIST)/$(CPSS_VERSION_LIB)

#$D/LDFLAGS := -lpcap $(LIBGCOV) -lutil -lpython2.7 -lpthread 
$D/LDFLAGS := $(SHAREDLIBSOPTS) $(LIBGCOV) -lutil -lpython2.7 -lpthread 
$D/LDFLAGS += -rdynamic -ldl -lrt -lcrypt

$D/all: $(DIST)/xpSaiApp $(DIST)/xpUnitTest
$D/CXX = $(CROSS_COMPILE)gcc  ${XP_SYSROOT}

################################################################################
# xpSaiApp Target
##############################################################################
$(DIST)/xpSaiApp: D:=$D
$(DIST)/xpSaiApp: $$($D/OBJS) $$($(WM)/exec/OBJS) $(LIBS_NEEDED_IN_ORDER) $$(MKFS)| $$(@D)/.
	$(QUIET)$(ECHO) $(LD) $@
#	$(QUIET)$(LD) -o $@  $(filter %.o, $^) -Wl,--start-group $(LIBS_NEEDED_IN_ORDER) -Wl,--end-group $($D/LDFLAGS) -Wl,--allow-multiple-definition
	$(LD) -o $@ $(filter %.o, $^) -Xlinker --whole-archive -Wl,--start-group $(LIBS_NEEDED_IN_ORDER)  $(CPPS_LIBS_IN_ORDER) -Wl,--end-group -Xlinker --no-whole-archive  $($D/LDFLAGS) 
	@$(ECHO)
	@$(ECHO) "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	@$(ECHO) "+                                                                              +"
	@$(ECHO) "+                           xpSaiApp  BUILD SUCCESSFUL!                            +"
	@$(ECHO) "+                                                                              +"
	@$(ECHO) "+                          The binary ($@) is ready!                   +"
	@$(ECHO) "+                                                                              +"
	@$(ECHO) "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	@$(ECHO)

$(DIST)/xpSaiAppTest: D:=$D
$(DIST)/xpSaiAppTest: $$($D/OBJS) $$($(WM)/exec/OBJS) $(LIBS_NEEDED_IN_ORDER) $(DIST)/libsaiShell.a $$(MKFS)| $$(@D)/.
	$(QUIET)$(ECHO) $(LD) $@
#	$(QUIET)$(LD) -o $@  $(filter %.o, $^) -Wl,--start-group $(LIBS_NEEDED_IN_ORDER) -Wl,--end-group $($D/LDFLAGS) -Wl,--allow-multiple-definition
	$(LD) -o $@ $(filter %.o, $^) -Xlinker --whole-archive -Wl,--start-group $(LIBS_NEEDED_IN_ORDER) $(DIST)/libsaiShell.a  $(CPPS_LIBS_IN_ORDER) -Wl,--end-group -Xlinker --no-whole-archive  $($D/LDFLAGS) 
	@$(ECHO)
	@$(ECHO) "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	@$(ECHO) "+                                                                              +"
	@$(ECHO) "+                           xpSaiAppTest  BUILD SUCCESSFUL                     +"
	@$(ECHO) "+                                                                              +"
	@$(ECHO) "+                          The binary ($@) is ready!                           +"
	@$(ECHO) "+                                                                              +"
	@$(ECHO) "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	@$(ECHO)



$(DIST)/xpSaiApp.so: $$(lib_OBJS.xpSaiApp) $$(MKFS)| $$(@D)/.
	$(QUIET)$(ECHO) $(CXX) $@
	$(QUIET)$(CXX) -I. $(GCXXFLAGS) $(INCLUDES) $(INCLUDES.$*_py) -fPIC \
		$(filter %.o, $^) $(filter %.a, $^) \
		$(LIBGCOV) -rdynamic -Xlinker -export-dynamic -shared -lrt $(SHAREDLIBSOPTS) -o $@

foobar: $(DIST)/_xpSaiApp.so xpSaiApp/$(MAINFILE)
#	g++ $(GCXXFLAGS) $($D/INCLUDES) -o $@ $^


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
