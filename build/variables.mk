#
# This makefile contains variable definitions for the main makefile
#
# Please consult with the Software Team before making changes
#

VARS_BEFORE_VARIABLES.MK := $(.VARIABLES)

################################################################################
# TOP of Marvell files used to build software
################################################################################
XP_TOP := $(abspath $(if $(findstring /xp80/, $(CURDIR)),..,../xp80))

# if XP_ROOT_REL or XP_ROOT was not set on command line, figure it out here
# if we are under xp80, include that
# if standalone, do not
# $(SUBDIR) was exported to us as where wrapper "maker" was called from
ifdef XP_ROOT_REL
export XP_ROOT ?= $(abspath $(SUBDIR)/$(XP_ROOT_REL))
else
export XP_ROOT ?= $(XP_TOP)
endif

ifneq ($(SYSROOT_PATH),)
	SYSROOT = $(SYSROOT_PATH)
endif

# Platform that make is being run on
UNAME      := $(shell uname)

ifeq ($(UNAME),Darwin)
MAC := Y
endif
ifeq ($(SYSROOT),)
	XP_SYSROOT =
else
	XP_SYSROOT =  --sysroot=${SYSROOT}
endif

ifeq ($(LIBC),uClibc)
	UCLIBC := Y
endif

# Toolchain to use - For now use the standard g++ located on your local machine
CC         := $(CROSS_COMPILE)gcc ${XP_SYSROOT}
CXX        := $(CROSS_COMPILE)g++ ${XP_SYSROOT}
STRIP      := $(CROSS_COMPILE)strip
OBJCOPY    := $(CROSS_COMPILE)objcopy

ifeq ($(XDK_DISABLE_DBG), 1)
        DBG = 0
endif

XDK_ENABLE_O2 = 1

$(info    XDK_ENABLE_O2 is $(XDK_ENABLE_O2))

# CFLAGS to use globally 
# -Wsizeof-pointer-memaccess is an overzealous compiler warning, nuke it
ifeq ($(XDK_ENABLE_O2), 1)
        GCXXFLAGS := -Wall $(if $(filter-out 0, $(DBG)), -g)  ${CFLAGS} -fPIC $(if $(filter-out 0, $(SM)), -Os) ${COVERFLAGS} -fno-strict-aliasing -O2 -D_GLIBCXX_USE_CXX11_ABI=0
else  
        GCXXFLAGS := -Wall $(if $(filter-out 0, $(DBG)), -g)  ${CFLAGS} -fPIC $(if $(filter-out 0, $(SM)), -Os) ${COVERFLAGS} -O0 -D_GLIBCXX_USE_CXX11_ABI=0
endif

ifeq ($(TARGET),a385)
    HARD_FLOAT_FLAGS := -mfloat-abi=hard -fPIC
endif


ifndef DISABLE_WARNING
   GCXXFLAGS += $(HARD_FLOAT_FLAGS) -Werror 
   CPPFLAGS += $(HARD_FLOAT_FLAGS) -Werror
   CFLAGS += $(HARD_FLOAT_FLAGS) -Werror -Wno-cpp #-Wno-cpp added to  suppress #warning directive in cpss.
endif

ifdef XP_WM_HIGH_SPEED
        GCXXFLAGS += -O3 -DDISABLE_LOG=1 -Wno-error=unused-result -Wno-error=unused-variable -Wno-error=unused-but-set-variable
endif

#Note: Uncomment below line if user wants fetch init time taken by various modules of XDK
#GCXXFLAGS += -DBOOT_TIME_STAT

# Note: Uncomment below line (within *UXDK flag) to communicate over TCP/UDP with simulator on the same machine or it will use shared memory
# In case of kernel SDK, we use there is only Netlink support which requires this flag to be set
#GCXXFLAGS += -DCPU_RX_TX_OVER_IPC

# To enable WM comment out below line
#XDK_FLAGS += -DDISABLE_WM

GCXXFLAGS += -DCAVIUM_KXDK


#Enable below lines for other than ksaiShell_py
ifneq ($(TARGET_APP), ksaiShell_py)
	GCXXFLAGS += -DKERNEL_SAISHELL
	KERNEL_SAISHELL := Y
endif

ifeq ($(DEVICES),)
	#GCXXFLAGS += -DALL_XPDEV
	#DEVICES = XP80A0 XP80B0 XP70A0 XP70A1 XP70A2 XP60 XP100
	DEVICES = XP80A0 XP80B0 XP70A0 XP70A1 XP70A2 XP60
	XP80A0 :=Y
	XP80B0 :=Y
	XP70A0 :=Y
	XP70A1 :=Y
	XP70A2 :=Y
	#XP100 :=Y
	XP60 :=Y
endif

# Define Device Flags for Device Type
ifeq ($(findstring XP80A0, $(DEVICES)),XP80A0)
	GCXXFLAGS += -DDXP80 -DDXP80A0
	XP80A0 :=Y
endif

ifeq ($(findstring XP80B0, $(DEVICES)),XP80B0)
	GCXXFLAGS += -DDXP80 -DDXP80B0
	XP80B0 :=Y
endif

ifeq ($(findstring XP70A0, $(DEVICES)),XP70A0)
	GCXXFLAGS += -DDXP70 -DDXP70A0
	XP70A0 :=Y
endif

ifeq ($(findstring XP70A1, $(DEVICES)),XP70A1)
	GCXXFLAGS += -DDXP70 -DDXP70A1
	XP70A1 :=Y
endif

ifeq ($(findstring XP70A2, $(DEVICES)),XP70A2)
	GCXXFLAGS += -DDXP70 -DDXP70A2
	XP70A2 :=Y
endif

ifeq ($(findstring XP100, $(DEVICES)),XP100)
	GCXXFLAGS += -DDXP100
	XP100 :=Y
endif

ifeq ($(findstring XP60, $(DEVICES)),XP60)
	GCXXFLAGS += -DDXP70 -DDXP70A0 -DDXP70A1 -DDXP70A2 -DDXP60
	XP60 := Y
	XP70A0 :=Y
	XP70A1 :=Y
	XP70A2 :=Y
endif

# GCC Version
GCCVERSION := $(shell expr `$(CXX) -dumpversion | cut -f1 -d.` \>= 4.6.0)
GIT_XDK_BRANCH := $(shell git rev-parse --abbrev-ref HEAD)
GIT_XDK_COMMIT := $(shell git rev-parse --short HEAD)
GCXXFLAGS  += -DGIT_XDK_BRANCH=$(GIT_XDK_BRANCH) -DGIT_XDK_COMMIT=$(GIT_XDK_COMMIT)

ifdef DISABLE_WARNING
	GCXXFLAGS  += -Werror -Wno-error=unused-but-set-variable  -Wno-unused-function -Wno-error=unused-function
else
	GCXXFLAGS  += -Werror
endif

# Multi-threading
ifeq ($(ENABLE_MULTITHREAD), 1)
	GCXXFLAGS  += -DXP_MT_ENABLED
	GCXXFLAGS  += -DXP_PL_LOCK_ENABLED -DXP_FL_LOCK_ENABLED -DXP_XPS_LOCK_ENABLED
else

ifeq ($(ENABLE_PL_LOCKS), 1)
	GCXXFLAGS  += -DXP_MT_ENABLED
	GCXXFLAGS  += -DXP_PL_LOCK_ENABLED
endif

ifeq ($(ENABLE_FL_LOCKS), 1)
	GCXXFLAGS  += -DXP_MT_ENABLED
	GCXXFLAGS  += -DXP_FL_LOCK_ENABLED -DXP_XPS_LOCK_ENABLED
endif

ifeq ($(ENABLE_XPS_LOCKS), 1)
	GCXXFLAGS  += -DXP_MT_ENABLED
	GCXXFLAGS  += -DXP_PL_LOCK_ENABLED -DXP_FL_LOCK_ENABLED -DXP_XPS_LOCK_ENABLED
endif
endif # ENABLE_MULTITHRED

ifeq ($(NO_TRACE_TIME), 1)
	GCXXFLAGS  += -DXP_WM_LOG_TIME=0
endif

ifeq ($(NO_TRACE_THREAD), 1)
	GCXXFLAGS  += -DXP_WM_TRACE_THREAD=0
endif

ifndef COMPRESSED_ADDR
   GCXXFLAGS += -DCOMPRESSED_ADDR
endif

ifeq ($(TARGET),native)
  GCXXFLAGS += -DASIC_SIMULATION
endif

ifeq ($(TARGET),native)
  GCXXFLAGS += -UPCAP_SUPPORT
endif

SAI ?= 1.8.1

$(info    SAI is $(SAI))
ifeq ($(SAI),1.7.1)
  GCXXFLAGS += -DSAI_1_7_1
  SAI_COMMITID = b908a5b
endif

ifeq ($(SAI),1.8.1)
  GCXXFLAGS += -DSAI_1_8_1
  SAI_COMMITID = 7cd3a7e
endif

ifeq ($(SAI),1.9.1)
  GCXXFLAGS += -DSAI_1_9_1
  SAI_COMMITID = 740a487
endif

SAI_PATH ?= "../plugins/sai/SAI/xpSai/sai"
$(eval GIT_COMMIT = $(shell cd $(SAI_PATH) && git checkout $(SAI_COMMITID)))

# For CPSS
LUA ?= yes
#DEBUG ?= no
DEMO ?= no

ifeq ($(TARGET),native)
ARCH ?= x86
endif
ifneq ($(TARGET),x86_64)
  GCXXFLAGS += -DRESET_PP_EXCLUDE_PEX
endif

GCXXFLAGS += -DCHX_FAMILY -DCAVIUM_MARVELL
#ifeq ("$(DEBUG)","yes")
#    DEBUG_GCXXFLAGS = -g -O0
#    export DEBUG_INFO=D_ON
#    GCXXFLAGS += -DDEBUG
#else
#    DEBUG_GCXXFLAGS = -O2
#    export DEBUG_INFO=D_OFF
#endif
#GCXXFLAGS += $(DEBUG_GCXXFLAGS)

ifeq ($(XDK_ENABLE_O2), 0)
GCXXFLAGS += -DDEBUG
endif


ifeq ("$(ARCH)", "x86")
  GCXXFLAGS += -DASIC_SIMULATION
endif

ifeq ("$(LUA)", "yes")
  GCXXFLAGS += -DSHELL_ENABLE
endif

ifeq ("$(DEMO)", "yes")
  GCXXFLAGS += -DDEMO
endif

GCXXFLAGS += -DCPSS_LOG_ENABLE

# Linker 
LD         := $(CROSS_COMPILE)g++ ${XP_SYSROOT}
#endif

# Archiver for static libs
AR         := $(CROSS_COMPILE)ar

# Archiver flags
ARFLAGS    := rcs

# file extensions compiled by the toolchain
CC_XT := cpp cxx c S

# Remove command
RM         := /bin/rm
CP         := /bin/cp

# Echo to console
ECHO       := /bin/echo

# Make a new directory
MKDIR      := /bin/mkdir

# Cat to shell
CAT        := /bin/cat

# Absolute path to top-level software directory
TOP        := $(CURDIR)/..

# Path to python include directory
# PYTHONINC := /usr/include/python2.7
PYTHONINC := ${SYSROOT}/usr/include/python2.7


ifdef undefined
# Env and SW Common Include directory
ifeq ($(ENVCOMMON),)
	ENVCOMMON   := common/swverif/include
endif
endif

# Shared Libraries (.so)
ifdef MAC
	SHAREDLIBSOPTS := 
	LIBGCOV        :=
else
ifeq ($(TARGET),native)
        SHAREDLIBSOPTS := -lpcap
else
        SHAREDLIBSOPTS :=
endif
	LIBGCOV        := -lgcov
endif

REL_TOP := ..
WM := $(REL_TOP)/whitemodel
MACH := $(shell getconf LONG_BIT)
ifeq ($(findstring Y, $(XP80A0) $(XP80B0)),Y)
#LIBXPWM := libXp80Wm_$(MACH).a
LIBXPWM := 
endif
ifeq ($(findstring Y, $(XP70A0) $(XP70A1) $(XP70A2)),Y)
LIBXP70WM := libXp70Wm_$(MACH).a
endif
#ifdef XP100
#LIBXP100WM := libXp100Wm_$(MACH).a
#endif
#LIBXPAAPL := libaapl-BSD.a
#LIBXPAAPLUTIL := libAaplUtil.a

LIBXPAAPL := 
LIBXPAAPLUTIL := 
#DEVICE_TYPE:= xp70
# Whitemodel Common Include directory
WMCOMMON    ?= $(WM)/common/include

CONFIG_DIR_PATH := "$(CURDIR)/$(WM)/devices/xp80/mgmt/config"
ABSOLUTE_DIR_PATH := $(TOP)

################################################################################
# command to list all the source control files in directories or file list $1, on stdout
# the directories are paths relative to where the command is invoked
# the source control files are listed relative to that too
################################################################################
SRC_CTRL_FILES_IN = git ls-files $1


LBITS := $(shell getconf LONG_BIT)


################################################################################
# directory for product distribution components
################################################################################
DIST := $(REL_TOP)/dist/${TARGET}

################################################################################
# directory for build artifact components
################################################################################
OBJSDIR := objs/${TARGET}/dummy

################################################################################
# Default Include paths
################################################################################
INCLUDES   := \
	-iquote $(REL_TOP)/$(WMCOMMON) \
	-iquote $(REL_TOP)/$(COMM)/include \

# CPSS INCLUDES PATH
CPSS_INCLUDES   := \
	-iquote $(REL_TOP)/cpss/common/h \
	-iquote $(REL_TOP)/cpss/common/h/cpssCommon \
	-iquote $(REL_TOP)/cpss/common/h/cpss/extServices/os/gtOs \
	-iquote $(REL_TOP)/cpss/mainPpDrv/h/cpss/dxCh/dxChxGen/bridge \
	-iquote $(REL_TOP)/cpss/mainPpDrv/h/cpss/dxCh/dxChxGen/ip \
	-iquote $(REL_TOP)/cpss/cpssEnabler/mainOs/h/gtOs      \
	-iquote $(REL_TOP)/cpss/cpssEnabler/cpssAppUtils/h \
	-iquote $(REL_TOP)/cpss/mainPpDrv/h/cpss/dxCh/dxChxGen/port/ \
	-iquote $(REL_TOP)/cpss/mainPpDrv/h/cpss/generic/	\
	-iquote $(REL_TOP)/cpss/common/h/cpssDriver/pp/hardware/ \
	-iquote $(REL_TOP)/cpss/mainPpDrv/h/cpss/dxCh/dxChxGen/networkIf/ \
    -iquote $(REL_TOP)/cpss/mainPpDrv/h/cpss/dxCh/dxChxGen/bridgeFdbManager/ \
    -iquote $(REL_TOP)/cpss/mainPpDrv/h/cpss/dxCh/dxChxGen/config/ \
	-I $(REL_TOP)/cpss/common/h/cpss/extServices/os \
	-I $(REL_TOP)/cpss/cpssEnabler/mainOs/h/ \
	-I $(REL_TOP)/cpss/cpssEnabler/mainExtDrv/h/ \
	-I $(REL_TOP)/cpss/cpssEnabler/mainCmd/h \
	-I $(REL_TOP)/cpss/cpssEnabler/cpssAppUtils/h \
	-I $(REL_TOP)/cpss/mainPpDrv/h/ \
	-I $(REL_TOP)/cpss/common/h/ \
	-I $(REL_TOP)/cpss/simulation/simGeneral/h/ \
	-I $(REL_TOP)/cpss/simulation/simOs/h/ \
	-I $(REL_TOP)/cpss/mainExtUtils/h/ \
	-I $(REL_TOP)/cpss/common/h/cpss/extServices/private \
	-I $(REL_TOP)/cpss/cpssEnabler/mainSysConfig/h/ \
	-I $(REL_TOP)/cpss/mainPpDrv/h/cpss/dxCh/dxChxGen/ip/ \
	-I $(REL_TOP)/cpss/mainPpDrv/h/cpss/dxCh/dxChxGen/networkIf/ \
	-I $(REL_TOP)/cpss/mainPpDrv/h/cpss/dxCh/dxChxGen/cos \
	-I $(REL_TOP)/cpss/mainPpDrv/h/cpss/dxCh/dxChxGen/cscd

ifneq ($(CPSS_MPD), MTD_NO)
    CPSS_INCLUDES  += \
        -I $(REL_TOP)/cpss/cpssEnabler/mainPhy/MPD/h/ \
        -I $(REL_TOP)/cpss/cpssEnabler/mainPhy/MPD/external_phy/mtd/src/ \
        -I $(REL_TOP)/cpss/cpssEnabler/mainPhy/MPD/external_phy/alaska_c/src
endif


################################################################################
#escaping %
################################################################################
PRC:=%


################################################################################
# all non-generated makefiles
# if variables in makefiles are at the top and then rules
# then the variable $(MAKEFILE_LIST) shows all the makefiles a rule could depend on
# since we want to evaluate it during parsing of the rule, it is deferred evaluation
# MKFS is used in prerequisite lists so, 
# MKFS= can be used on command line to disregard dependency on makefiles
################################################################################
MKFS = $(sort $(filter %Makefile %.mk, $(MAKEFILE_LIST)))

################################################################################
# duplicate each variable FOOBAR to /FOOBAR
# this is so that we can use $P/FOOBAR for empty parent $P
################################################################################
$(foreach var,$(filter-out $(VARS_BEFORE_VARIABLES.MK), $(.VARIABLES)),$(eval /$(var) := $($(var))))


################################################################################
# include all files dir.mk from $1
# $1 contains subdirectories relative to where this call is written
# sets DIR variable to use in $(DIR)/dir.mk
#
# do not allow evil /.. in dir.mk
################################################################################
INCL_SUBDIRS = $(eval $(foreach subdir,$1,$(call INCL_SUBDIR,$(subdir))))

define INCL_SUBDIR

DIR := $1
ERR := $$(shell grep -s /\\.\\. $$(DIR)/dir.mk)
$$(if $$(ERR), $$(error /.. not allowed in $$(DIR)/dir.mk: $$(ERR)))
-include $$(DIR)/dir.mk

endef


################################################################################
# make so that you can evaluate these variables correctly from within recipes
# inside recipes is deferred evaluation, so we store the current value of such a variable in a 
# target-specific variable
################################################################################
TARGET_VARS := \
	D \
	P \
	SUBDIR_FILELISTS

$(foreach var,$(TARGET_VARS),$(eval %: $(var) := $($var)))


################################################################################
# this must be evaluated at the top of each dir.mk
################################################################################
TOP_MAGIC = $(eval include $(CURDIR)/dir_top.mk)

################################################################################
# this must be evaluated at the bottom of each dir.mk
################################################################################
BOTTOM_MAGIC = $(eval include $(CURDIR)/dir_bottom.mk)


################################################################################
# base part of the *.mk filename
################################################################################
FILENAME_BASE = $(patsubst %.mk,%, $(notdir $(lastword $(MAKEFILE_LIST))))


################################################################################
# depend one product $1 in products/product.mk file, on other such products $2
# 
# depend as make targets
# and add deferred to SUBDIRS.product , their subdirs
#
# DEPEND_PRODUCT can be just used as 
#
# $(call DEPEND_PRODUCT,foobar, foobar1 foobar2)
# 
# (without $(eval))
#
# notice there must be no space before first argument
################################################################################

define DEPEND_PRODUCT_FUNCTION
$1: $2

SUBDIRS.$1 = $(foreach prod, $2, $(SUBDIRS.$(prod)))
endef

define DEPEND_PRODUCT
	$(eval $(call DEPEND_PRODUCT_FUNCTION,$1,$2))
endef


################################################################################
# GNU Make canned (partial) recipe, to use when you want to always attempt to update a make target, but don't touch it if 
# there were no changes in contents 
# (so that further updates are not triggered if there is no change to contents of this target)
#
# use like this:
# target: phony
#	recipe $(CHANGE_TARGET_ONLY_IF_OUT_DIFF)
#
# that is:
# after the recipe that outputs the new target on stdout, put this canned fragment
# temporary file is then created from stdout and compared with the existing target, only if they do not match, the
# target is replaced
################################################################################
define CHANGE_TARGET_ONLY_IF_OUT_DIFF =
	>$@.temp
	if ! diff $@.temp $@ ; \
		then mv $@.temp $@ ; \
	fi \
	2>/dev/null; true
endef


CC_XT_FILTER := $(addprefix %., $(CC_XT))

CC_SRCS = $(filter $(CC_XT_FILTER),$1)

OBJS_FROM_CC_SRCS = $(foreach src, $1,$(OBJSDIR)/$(basename $(src)).o)


################################################################################
# rule to combine lists of source files and object C-header dependency makefiles, into one list of files
#
# $2 are files, each is a list of C source files, one on a line
# $3 are source files, 
# some of which are compiled by gcc, where there is a corresponding .d auto-generated dependency makefile
# so in there there are also source files mentioned (headers)
# this rule combines all the source files mentioned above, into the target $1
#
# $2 or $3 may be empty, but not both
#
# this rule depends on deferred-evaluation variable $$(MKFS) which is all the makefile files
# and uses $(CAT) which is a utility that shows files on stdout
#
# to establish the rule, this function has to be called as
# $(eval $(call COMBINE_SRCS_TO_FILELISTS ...))
# 
# that is why inside make function evaluations use escaped $$
# and then the recipe shell special character $ has to be recursively escaped as $$$$
#
# for files in $3, if the file is not compiled by gcc toolchain, we just list that file, 
# if it is, we parse the dependency makefile, looking for files that we list
#
# we have to be very careful to use pipes correctly to not allow arbitrarily long command lines, which could easily exceed
# allowed length if the filesystem grows large
# 
# we use some tricky formatting of the .d file coming out of GCC -MP option:
# there are words there ending with ":" - namely the object file itself, and duplicates of header files, so we can ignore those
# also ignore paths not relative to us (starting with /)
# and the rest are words consisting of "\" or longer ones ending with "\", when that is echoed, the trailing single backslash is 
# deleted, lucky for us
#
# in case somebody is using the evil /.. , we canonicize the relative path by piping through $(ABS_PATH) 
# exported from maker script, then stripping the starting $(CURDIR) back again
################################################################################
define COMBINE_SRCS_TO_FILELISTS

$1: $2 $(call OBJS_FROM_CC_SRCS,$(filter $(CC_XT_FILTER),$3)) $$(MKFS)
	(:; \
		$$(if $2,$(CAT) $2;) \
		$$(foreach file, $3,$$(if $$(filter $(CC_XT_FILTER),$$(file)),\
			($(CAT) $(OBJSDIR)/$$(basename $$(file)).o.d | \
			while read line; do \
				for token in $$$$line; do \
					if [[ ! $$$$token =~ :$$$$ && ! $$$$token =~ ^/ ]]; \
					then \
						string=`$(ABS_PATH) $$$$token`; \
						echo $$$${string#$(CURDIR)/}; \
					fi; \
				done; \
			done); \
		,\
		echo $$(file); \
		)) \
	) | sort -u > $$@

endef


################################################################################
# implicit rule, with pattern being a product, denoted by % :
# combine filelists filelist.$1.%.txt in SUBDIRS.% below, to the same filelist in this directory
#
# $1 is a type of filelist, for example 'customer'
#
# we also add $(SINGLE_SRCS.<product>), filtered by filter function $2 if any

#
# the target is filelist.%.$1.txt
# 
# the prerequisites are the same files in all top-level subdirectories of the product: $(SUBDIRS.%)
# we also depend on this makefile
#
# the filelists list source file one on a line
# the rule combines the prerequisite filelists and outputs to target without repetitions\
#
# 
# the filtering is because for some type of filelist, we may not want to include all of the $(SINGLE_SRCS...)
#
# we assume $(SUBDIRS.%) may be established later, so do .SECONDEXPANSION:
#
# this function will be called as 
# $(eval $(call COMBINE_PRODUCT_FILELISTS_BELOW,type,optional filter function))
# and SUBDIRS... prerequisites are already escaped $$ deferred evaluation
# so we need to recursively escape that for eval function
################################################################################
define COMBINE_PRODUCT_FILELISTS_BELOW

.SECONDEXPANSION:

# type extension to the file names
TYPE_XT := $(if $1,.$1)

# make above TYPE_XT valid in the recipe below
filelist.%$$(TYPE_XT).txt: TYPE_XT := $$(TYPE_XT)

filelist.%$$(TYPE_XT).txt: $$$$(addsuffix /filelist$$(TYPE_XT).txt, $$$$(SUBDIRS.$$$$*)) variables.mk
	( \
	for token in \
		$$(if $2, $$(call $2, $$(SINGLE_SRCS.$$*)), $$(SINGLE_SRCS.$$*)) \
	; do \
		echo $$$$token; \
	done; \
	$(CAT) $$(filter %.txt, $$^) \
	) \
	| sort -u > $$@

endef


################################################################################
#define a phony target $1/make for directory $1
#so that you can depend on that and have it call Make there
#
# apparently Make can't determine whom it is calling recursively, so we start recipe with '+' to correctly pass options like -j
################################################################################
define PHONY_MAKE_TARGET

.PHONY: $1/make
$1/make:
	+$(MAKE) -C $1 ; \
	echo 1 is $1 '************* D E B U G *********** debug' 

endef


# by default we do know how to autogenerate source files
GEN := 1


################################################################################
# SWIG warnings
################################################################################
SWIGWARN_PARSE_NESTED_CLASS := 312#
SWIGWARN_IGNORE_OPERATOR_EQ := 362#


################################################################################
# this function called in the context of a $(PRODUCT).mk file, on a prerequisite line:
#
# target: $(call OBJS_FROM_PROD_DIRS_CC_SRCS,FOO_SRCS)
# or if it had, keep the same value
# if the new value were not the same, bail out with an error
#
# collects in each directory D of SUBDIRS.$(PRODUCT)
# the sources $D/FOO_SRCS
# and returns their objects
#
# in this way you obtain the list of objects ready on the prerequisite line
# and you can use them in the recipe something like this
# $(filter %.o, $^)
################################################################################
OBJS_FROM_PROD_DIRS_CC_SRCS = $$(call OBJS_FROM_CC_SRCS, $$(foreach dir, $(SUBDIRS.$(PRODUCT)),$$($$(dir)/$1) ))

################################################################################
# canned recipe to make a shared library from all .o files on the prerequisite line
################################################################################
define MAKE_SHARED_LIB =
	$(QUIET)$(ECHO) $(CXX) $@
	$(QUIET)$(CXX) -fPIC -rdynamic $(filter %.o, $^) ${LDFLAGS} -shared -o $@
endef

################################################################################
# canned recipe to make a shared library from all .o files on the prerequisite line
################################################################################
define MAKE_SHARED_LIB_C =
	$(QUIET)$(ECHO) $(CC) $@
	$(QUIET)$(CC) -fPIC -rdynamic $(filter %.o, $^) ${LDFLAGS} -shared -o $@
endef

################################################################################
# canned recipe to make a static library from all .o files on the prerequisite line
################################################################################
define MAKE_STATIC_LIB =
	$(QUIET)$(RM) -f $@
	$(QUIET)$(ECHO) $(AR) $@
	$(QUIET)$(AR) $(ARFLAGS) $@ $(filter %.o, $^)
endef

################################################################################
# assign value $2 to variable $1 "once":
#
# if $2 has not yet been assigned, then assign
# or if it had, keep the same value
# if the new value were not the same, bail out with an error
#
# disregard differences by whitespace only
#
# this function is meant to be called from within an eval function, 
# that is why the error message is deferred evaluated, so it does not bail during the expansion of the call
################################################################################
define ASSIGN_ONCE
ifeq '$(origin $1)' 'undefined'
$1 := $(strip $2)
else
ifneq '$($1)' '$(strip $2)'
$$(error WHOA!! cannot redefine the variable $1 from '$$($1)' to '$2')
endif
endif
endef


################################################################################
# $(call PROD_VAR,FOOBAR)
# made in the context of a directory $(D)
# evaluates to the value of FOOBAR.$(product)
# for any product that contains $(D) in its $(SUBDIRS).product
#
# this is to setup per-product variables
# 
# for example, if you define in products/sdk.mk :
# FOOBAR.sdk := foobar
# then you can propagate this variable to every subdirectory dir.mk in sdk product:
# in each such dir.mk, you can do:
# $(call PROD_VAR,FOOBAR)
# and this call will have a value, which is "foobar"
# 
# Note there is no space before or after FOOBAR in the call to PROD_VAR
#
# If more than one product has the same subdirectory dir.mk, 
# then any such variable must have the same value for each of these product
# otherwise we issue an error
#
#
# algorithm:
# we store the result in the variable _FOOBAR,
# the eval call to PROD_VAR_FUNCTION is purely side effects, returns nothing, so then
# we simply note and return that result in the end
#
# for each product which includes $(D), we "assign once" the value of FOOBAR.$(product) to _FOOBAR
#
# this variable name is chosen so that it is unique per FOOBAR, so that the state of not-yet-assigned which is important 
# for the function ASSIGN_ONCE, is achieved once per FOOBAR
# at the same time, we want a name that is not likely to conflict with other names, hence the underscore
#
# we order the arguments of $(filter) "strangely" because it is faster this way
################################################################################

define PROD_VAR_FUNCTION
TEMP := _$1
$$(foreach prod, $(MOD_PRODUCTS), $$(if $$(filter $$(SUBDIRS.$$(prod)),$(D)), \
	$$(eval $$(call ASSIGN_ONCE,$$(TEMP),$$($1.$$(prod))))))
endef

PROD_VAR = $(eval $(call PROD_VAR_FUNCTION,$1)) $(_$1)


################################################################################
# canned recipe to copy files from $1 that exist to directory $2, including creating $2
################################################################################
define COPY_FILES_TO_DIR
mkdir -p $2; \
for file in $(wildcard $1) ; \
do \
	mkdir -p $2/$$(dirname $$file); \
	cp $$file $2/$$file; \
done
endef
