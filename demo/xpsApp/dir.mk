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
	-iquote $(REL_TOP)/demo/xpsApp/include \
	-iquote $(REL_TOP)/demo/xpsApp/l2/include \
	-iquote $(REL_TOP)/demo/xpsApp/l3/include \
	-iquote $(REL_TOP)/demo/xpsApp/tunnel/include \
	-iquote $(REL_TOP)/demo/xpsApp/qos/include \
	-iquote $(REL_TOP)/demo/xpsApp/copp/include \
	-iquote $(REL_TOP)/xps/include \
	-iquote $(REL_TOP)/utils/sal/include \
	-iquote $(REL_TOP)/utils/state/include \
	-iquote $(REL_TOP)/demo/common/include \
	-iquote $(REL_TOP)/demo/config/include \
	-iquote $(REL_TOP)/demo/common/py \
	-I$(REL_TOP)/cpssHal/cpssHalInit/include \
    -I $(REL_TOP)/cpssHal/cpssHalDeviceMgr/include \
	-I$(REL_TOP)/cpssHal/cpssHalShell/include \
	-I$(REL_TOP)/cpssHal/cpssHalSim/include \
	-I$(REL_TOP)/cpssHal/cpssHalUtil/include/ \
	$(CPSS_INCLUDES) \
	-iquote $(REL_TOP)/platform/include 

ifdef MAC
	$D/INCLUDES += -I../lib
endif

GCXXFLAGS += -DSHELL_ENABLE

$D/CXXFLAGS  := $(GCXXFLAGS) $(call PROD_VAR,CXXFLAGS) $($D/INCLUDES) -Wno-deprecated-declarations

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
    $D/xpsApp.c \
    $D/l2/xpsAppL2.c \
    $D/l3/xpsAppL3.c \
    $D/xpsAppL3Uc.c \
    $D/tunnel/xpsAppTnl.c \
    $D/tunnel/xpsAppMpls.c \
    $D/qos/xpsAppQos.c\
    $D/copp/xpsAppCopp.c

$D/SRCS     := \
	$($D/LIB_SRCS) \
	$D/xpsAppMain.c


#################################################################################
# libraries needed for our executable, in order
################################################################################
LIBS_NEEDED_IN_ORDER_XPS :=
LIBS_NEEDED_IN_ORDER_UNITTEST :=

LIBS_NEEDED_IN_ORDER_XPS += \
	$(DIST)/libApp.a \
	$(DIST)/libXpPy.a \
	$(DIST)/libXdkXps.a \
	$(DIST)/libXpsState.a \
	$(DIST)/libXpsAllocator.a \
	$(DIST)/libXpsSal.a
LIBS_NEEDED_IN_ORDER_UNITTEST += \
    $(LIBS_NEEDED_IN_ORDER_UNITTEST_WM) \
	$(DIST)/libXdk.a \



CPPS_LIBS_IN_ORDER := $(CPSS_LIBS)
CPPS_LIBS_IN_ORDER += $(DIST)/$(CPSS_ENABLER_LIB)
CPPS_LIBS_IN_ORDER += $(DIST)/$(CPSS_VERSION_LIB)

$D/LDFLAGS := -lpcap $(LIBGCOV) -lutil -lpython2.7 -lpthread


$D/LDFLAGS += -rdynamic -ldl -lrt

$D/all: $(DIST)/xpsApp
$D/CXX = $(CROSS_COMPILE)gcc  ${XP_SYSROOT}

################################################################################
# xpsApp Target
##############################################################################

$(DIST)/xpsApp: D:=$D
$(DIST)/xpsApp: $$($D/OBJS) $(LIBS_NEEDED_IN_ORDER_XPS) | $$(@D)/.
	$(LD) -o $@ $(filter %.o, $^) -Xlinker --whole-archive -Wl,--start-group $(LIBS_NEEDED_IN_ORDER_XPS)  $(CPPS_LIBS_IN_ORDER) -Wl,--end-group -Xlinker --no-whole-archive  $($D/LDFLAGS) 
	@$(ECHO) 
	@$(ECHO) "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	@$(ECHO) "+                                                                              +"
	@$(ECHO) "+                           xpsApp  BUILD SUCCESSFUL!                            +"
	@$(ECHO) "+                                                                              +"
	@$(ECHO) "+                          The binary ($@) is ready!                   +"
	@$(ECHO) "+                                                                              +"
	@$(ECHO) "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	@$(ECHO)


$(DIST)/xpsApp.so: $$(lib_OBJS.xpsApp) | $$(@D)/.
	$(CXX) -I. $(GCXXFLAGS) $(INCLUDES) $(INCLUDES.$*_py) -fPIC \
		$(filter %.o, $^) $(filter %.a, $^) \
		$(LIBGCOV) -rdynamic -Xlinker -export-dynamic -shared -lrt $(SHAREDLIBSOPTS) -o $@

foobar: $(DIST)/_xpsApp.so xpsApp/$(MAINFILE)


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
