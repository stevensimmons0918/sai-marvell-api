###########################################################
# Function definitions
###########################################################

#allow the caller to specify additional CFLAGS
#CPSS_USER_CFLAGS hold multiple 'CFLAGS' , defined as 'export CPSS_USER_CFLAGS="..."'
#example : export CPSS_USER_CFLAGS="-fPIC" before calling the 'make'
CFLAGS += $(CPSS_USER_CFLAGS)
VERSION_FLAGS := FAMILY=$(FAMILY)

ifeq ($(OS),Windows_NT)
SHELL = cmd.exe
MKDIR = mkdir
else
SHELL = /bin/bash
AWK = /usr/bin/awk
MKDIR = mkdir -p
SOURCE = source
endif

#---- uppercase function to convert string to uppercase ----------
uppercase_TABLE:=a,A b,B c,C d,D e,E f,F g,G h,H i,I j,J \
    k,K l,L m,M n,N o,O p,P q,Q r,R s,S t,T u,U v,V w,W x,X y,Y z,Z

define uppercase_internal
$(if $1,$$(subst $(firstword $1),$(call uppercase_internal,$(wordlist 2,$(words $1),$1),$2)),$2)
endef

define uppercase
$(eval uppercase_RESULT:=$(call uppercase_internal,$(uppercase_TABLE),$1))$(uppercase_RESULT)
endef
#-----------------------------------------------------------------


########################   Preperations  ##########################
###                                                             ###
###################################################################

# Name of makefile to recursively include in each sub-module
MAKEFILE := _Makefile

DEP :=

ECHO = echo
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
STRIP = $(CROSS_COMPILE)strip
CP = cp

# Prepend a few empty lines gap when calling the makefile
$(info  )
$(info  )

# Verify target is provided: armv7, armv8, sim32, sim64, ia64
# This is relevant only for targets: appDemo, kernel and rootfs
ifeq ("$(TARGET)", "")
ifeq ("$(MAKECMDGOALS)", "$(filter $(MAKECMDGOALS), appDemo kernel rootfs)")
  $(error TARGET setting missing)
endif
else
include tools/build/targets/$(TARGET).mk
endif

ifeq ($(OS_RUN), win32)
 TOOL_TYPE ?= vc
else
 TOOL_TYPE ?= gcc
endif

##########################   config    ############################
###                                                             ###
###################################################################
CONFIG ?= .config
# Include configuration file, if exists
-include $(CONFIG)

config_list := \
 CONFIG_INCLUDE_TM \
 CONFIG_INCLUDE_UTF \
 CONFIG_INCLUDE_GALTIS \
 CONFIG_SHARED_MEMORY \
 CONFIG_ASIC_SIMULATION \
 CONFIG_ASIC_SIMULATION_ENV_FORBIDDEN \
 CONFIG_INCLUDE_EZ_BRINGUP \
 CONFIG_GM \
 CONFIG_DEBUG_INFO \
 CONFIG_ASK \
 CONFIG_ASK_LIB_BUILD \
 CONFIG_DISABLE_CLI_PRINTS \
 CONFIG_EXT_MODULE \
 CONFIG_PTP_PLUGIN_SUPPOR \
 CONFIG_TRAFFIC_API_LOCK_DISABLE \
 CONFIG_USE_MUTEX_PROFILER \
 CONFIG_STACK_SIZE_LIMIT \
 CONFIG_LOG_ENABLE \
 CONFIG_INCLUDE_MPD \
 CONFIG_CMD_FS \
 CONFIG_CMD_LUA_CLI \
 CONFIG_API_LOCK_PROTECTION

# Parse command line parameters:
cli_config_list := $(patsubst CONFIG_%,%,$(config_list))
# If .config does not exist and parameter is set in command line, override defaults
ifeq (,$(wildcard $(CONFIG)))
 $(info $(CONFIG) deleted)
 $(foreach S,$(cli_config_list),$(if $($(S)),$(eval CONFIG_$(S) = $($(S)))))
else

 # Check if configuration has changed since last build
 # First case - item set to 'y' in .config, and declared 'n' in CLI
 cli_config := $(foreach S,$(cli_config_list),$(if $(filter $($(S))$(CONFIG_$(S)),ny),yes))
 # Second case - item not set in .config, and declared 'y' in CLI
 cli_config += $(foreach S,$(cli_config_list),$(if $(filter $($(S)),y),$(if $(filter $($(S))$(CONFIG_$(S)),y),yes)))
 # 3rd case - GM changed
 ifneq ($(CONFIG_GM),$(GM))
  cli_config += yes
 endif

 ifneq ($(strip $(cli_config)),)
  $(info $(CONFIG) changed)
  $(shell rm $(CONFIG))
 endif
 $(foreach S,$(cli_config_list),$(if $($(S)),$(eval CONFIG_$(S) = $($(S)))))
 # Display list of enabled flags
 config_print := $(shell grep y $(CONFIG))
 $(info $(config_print))
endif

# Defaults
CONFIG_LOG_ENABLE ?= y
CONFIG_CMD_FS ?= y
CONFIG_CMD_LUA_CLI ?= y
CONFIG_API_LOCK_PROTECTION ?= y

config := $(foreach S,$(filter-out CONFIG_GM,$(config_list)),$(if $(filter $($(S)),y),"echo $S=y;", "echo '\#' $S is not set;"))
# GM requires special care - not a y/n value
ifeq ($(GM),)
 config += "echo '\#' CONFIG_GM is not set;"
else
 config += "echo CONFIG_GM=$(GM);"
endif

#  Add flag per configuration item
CONFIG_CFLAGS-y :=
CONFIG_CFLAGS-$(CONFIG_INCLUDE_TM) += -DINCLUDE_TM
CONFIG_CFLAGS-$(CONFIG_LOG_ENABLE) += -DCPSS_LOG_ENABLE
CONFIG_CFLAGS-$(CONFIG_API_LOCK_PROTECTION) += -DCPSS_API_LOCK_PROTECTION
CONFIG_CFLAGS-$(CONFIG_SHARED_MEMORY) += -DPOSIX_SEM -DSHARED_MEMORY $(PIE)
CONFIG_CFLAGS-$(CONFIG_TRAFFIC_API_LOCK_DISABLE) += -DCPSS_TRAFFIC_API_LOCK_DISABLE
CONFIG_CFLAGS-$(CONFIG_USE_MUTEX_PROFILER) += -DCPSS_USE_MUTEX_PROFILER
CONFIG_CFLAGS-$(CONFIG_PTP_PLUGIN_SUPPORT) += -DPTP_PLUGIN_SUPPORT
CONFIG_CFLAGS-$(CONFIG_INCLUDE_UTF) += -DINCLUDE_UTF -DIMPL_TGF
CONFIG_CFLAGS-$(CONFIG_INCLUDE_EZ_BRINGUP) += -DEZ_BRINGUP
CONFIG_CFLAGS-$(CONFIG_INCLUDE_MPD) += -DMPD -DINCLUDE_MPD
CONFIG_CFLAGS-$(CONFIG_INCLUDE_GALTIS) += -DIMPL_GALTIS
CONFIG_CFLAGS-$(CONFIG_CMD_LUA_CLI) += -DCMD_LUA_CLI -DCMD_LUA_NET -DCMD_LUA_GLOBAL -DCMD_LUA_GENWRAPPER
CONFIG_CFLAGS-$(CONFIG_CMD_LUA_CLI) += -DCMDFS_RAM_EMBEDDED_DATA_EXT -DCMDFS_RAM_EMBEDDED_DATA_EXT_EXAMPLES
CONFIG_CFLAGS-$(CONFIG_CMD_LUA_CLI) += -DCMDFS_RAM_EMBEDDED_DATA_EXT_XML -DENV_MAINCMD

CONFIG_CFLAGS-$(CONFIG_ASIC_SIMULATION) += -DASIC_SIMULATION
CONFIG_CFLAGS-$(CONFIG_ASIC_SIMULATION_ENV_FORBIDDEN) += -DASIC_SIMULATION_ENV_FORBIDDEN
CONFIG_CFLAGS-$(CONFIG_CMD_FS) += -DCMD_FS

ifneq ($(CONFIG_GM),)
 CONFIG_ASIC_SIMULATION := y
 CONFIG_CFLAGS-y += -DGM_USED
 LDFLAGS-y += -lstdc++
endif
VERSION_FLAGS-y :=
VERSION_FLAGS-$(CONFIG_INCLUDE_TM) += INCLUDE_TM
VERSION_FLAGS-$(CONFIG_INCLUDE_UTF) += INCLUDE_UTF
VERSION_FLAGS-$(CONFIG_INCLUDE_GALTIS) += INCLUDE_GALTIS
VERSION_FLAGS-$(CONFIG_SHARED_MEMORY) += SHARED
VERSION_FLAGS-$(CONFIG_INCLUDE_EZ_BRINGUP) += EZ_BRINGUP
VERSION_FLAGS-$(CONFIG_GM) += GM
VERSION_FLAGS-$(CONFIG_INCLUDE_MPD) += INCLUDE_MPD
VERSION_FLAGS-$(CONFIG_TRAFFIC_API_LOCK_DISABLE) += TRAFFIC_API_LOCK_DISABLE
VERSION_FLAGS-$(CONFIG_USE_MUTEX_PROFILER) += USE_MUTEX_PROFILER
VERSION_FLAGS-$(CONFIG_CMD_LUA_CLI) += CMD_LUA_CLI




######################## compile ##################################
###                                                             ###
###  define compiler commands as per the TOOL_TYPE              ###
###                                                             ###
###################################################################

ifeq ($(OS), Windows_NT)
# compile using msvsc tools
# create list of include headers from VC environment variable "INCLUDE"
# This is used by makedepend
INCLUDE_VC_MD := -I"$(strip $(subst ;," -I",$(INCLUDE)))"

#-----------------------------------------------------------------
# Function compile(dep_cflags, compile_cflags, output_folder)
#
# Compile a .c file into .o file using the defined toolchain, including dependecy .d file.
# Object and dependency files will be located together in output_folder.
# cflags        - CFLAGS to use for creating object file
# output_folder - object file ouptut folder path prefix. Only files
#                 which should be created in this folder, will use
#                 this recipe.
#
# Example: $(call compile $(CFLAGS),/local/objects)
# Targets: /local/objects/foo/bar/baz.o
#          /local/objects/foo/bar/baz.d
# Source file: foo/bar/baz.c
#-----------------------------------------------------------------

define compile
	-if not exist $(dir $@) ($(MKDIR) $(subst /,\,$(dir $@)))
	$(if $(filter-out $(DEPENDENCY),DEP_NO),makedepend -f- -o.o -s# -p$(BUILD_FOLDER)/ -D_M_IX86 -D_WIN32 -- $(INCLUDE_VC_MD) $(CFLAGS) -- $< > $2/$*.d,)
	$(CL) -c $(VC_CFLAGS) $1 $< -$(O)o$@
endef

else
# compile using gcc tools
define compile
@# create folder for dependency file
	$(MKDIR) $(dir $@)
@# create dependency file
	$(if $(filter-out $(DEPENDENCY),DEP_NO),$(CC) -M -c $1 $< -o$2/$*.d,)
@# Duplicate it
	$(if $(filter-out $(DEPENDENCY),DEP_NO),cp $2/$*.d $2/$*.tmp,)
@# Create empty rule per file in dependency rule to avoid "No rule to make target ..." Errors
	$(if $(filter-out $(DEPENDENCY),DEP_NO),sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/d' -e 's/$$/ :/' < $2/$*.d >> $2/$*.tmp,)
@# Change object filename to include full path
	$(if $(filter-out $(DEPENDENCY),DEP_NO),sed -e 's/$(subst .,\.,$(notdir $@))/$(subst /,\/,$@)/'  < $2/$*.tmp > $2/$*.d,)
@# Remove temporary file
	$(if $(filter-out $(DEPENDENCY),DEP_NO),rm $2/$*.tmp,)
@# print compilation msg
	@ $(ECHO) CC $<
@# compile
	$(CL) -c $(VC_CFLAGS) $1 $< -$(O)o$@
endef

endif

ENABLER             := appDemo

# If it is a Cpss ASK(Application Starter kit)  build..
ifeq (y, $(CONFIG_ASK))
  LINUX_CPSS_APP_REF  := YES
  LINUX_CPSS_APP_LIB  := YES
  CFLAGS              += -DPOSIX_SEM -DCPSS_APP_PLATFORM

  ifeq ($(CONFIG_SHARED_MEMORY),y)
    BUILD_CPSS_APP_PLAT := EXE_SHARED
  else
    BUILD_CPSS_APP_PLAT := EXE
  endif

  ifeq (y, $(CONFIG_ASK_LIB_BUILD))
    BUILD_CPSS_APP_PLAT := LIB
  else
    CFLAGS += -DCPSS_APP_PLATFORM_REFERENCE
  endif
endif

ifeq ($(CONFIG_DISABLE_CLI_PRINTS), y)
    CFLAGS += -DDISABLE_CLI_SERVICE
endif

ifeq ($(DISABLE_SHELL_SERVICE), y)
    CFLAGS += -DDISABLE_SHELL_SERVICE
endif

ifeq ($(DISABLE_KEEPALIVE_SERVICE), y)
    CFLAGS += -DDISABLE_KEEPALIVE_SERVICE
endif

ifeq ($(REMOVE_CLI_PRINTS), y)
    CFLAGS += -DREMOVE_CLI_PRINTS
endif

# Figure out paths to various source and target folders
# #####################################################
MY_DIR = $(shell pwd)
VERSION_DATE = $(shell date "+%Y-%m-%d %T")

# List of targets for "help" printout
ifneq ($(OS),Windows_NT)
 targets := $(shell ls tools\/build\/targets | sed 's/\(.*\)\.mk/ \\*   \1/g')
else
 targets := $(shell dir /B tools\build\targets)
 targets := $(basename  $(targets))
endif

# Create build folder name according to build options, unless provided.
# By default, build folder is ./compilation_root
# Create sub-folders in the following format:
# compilation_root/branch/target_family_postfix,
# postfix describes build type (GM, shared, etc)
###############################################################
ifneq ("$(TARGET)", "")
# Default path to cpss build folder (compilation_root), considering git
# In case make is called with -C, compilation_root should be based at caller path
ifneq ($(CALLER_PATH),)
 override CALLER_PATH := $(CALLER_PATH)/
endif

ifeq ($(OS),Windows_NT)
 ISGIT := $(shell if exist .git echo Y)
 BUILD_FOLDER ?= $(CALLER_PATH)compilation_root
else
 ISGIT := $(shell git rev-parse --git-dir 2> /dev/null)
 # Check if freeRTOS folder exist
 IS_FREERTOS := $(shell ls extension/srvCpu/firmware/internal/build_free_rtos.sh 2>/dev/null)
endif

ifneq ($(ISGIT),)
 BRANCH := /$(shell git rev-parse --abbrev-ref HEAD)
endif
ifeq ($(CONFIG_SHARED_MEMORY), y)
 POSTFIX += _SHARED
endif
ifneq ($(CONFIG_GM),)
 POSTFIX += _GM_$(CONFIG_GM)
endif

ifneq ($(OS),Windows_NT)
 BUILD_FOLDER := $(CALLER_PATH)compilation_root$(BRANCH)/$(TARGET)_$(FAMILY)$(POSTFIX)
else
 override BUILD_FOLDER := $(BUILD_FOLDER)$(BRANCH)/$(TARGET)_$(FAMILY)$(POSTFIX)
endif
$(info BUILD_FOLDER = $(BUILD_FOLDER))
endif

# path to rootfs
ROOTFS_FOLDER=$(BUILD_FOLDER)/rootfs

# Information about kernel source / headers, and kernel modules.
# Relevant only for building kernel modules and / or kernel image.
KERNEL_FOLDER ?= ../kernel
MODULES_FOLDER := $(MY_DIR)/cpssEnabler/mainExtDrv/src/gtExtDrv/linuxNoKernelModule/drivers


############### Host specific settings ############################
###                                                             ###
###  Set compiler and corresponding flags for different hosts   ###
###                                                             ###
###################################################################

ifeq (y, $(CONFIG_ASIC_SIMULATION))
    simFilesUsed=1
endif
#check if we forbid the use of the simulation files ! even if the ASIC_SIMULATION is used !
ifeq (y, $(CONFIG_ASIC_SIMULATION_ENV_FORBIDDEN))
    simFilesUsed=0
endif

ifeq ($(TOOL_TYPE), vc)
 # Flags to support windows simulation based on Visual Studio
 ############################################################
 # cl is name of VC compiler
 CL := cl
 # cl expects -Fo to denote output (instead of -o)
 O := F
 LD := link
 AR = lib
else
 ##### Linux GNU GCC #####
 ifeq ($(CONFIG_DEBUG_INFO), y)
  CFLAGS += -g -O0
 else
  CFLAGS += -O2
 endif
 ifeq (1, $(simFilesUsed))
  CFLAGS += -Wno-unused-parameter -Wno-missing-field-initializers -Wno-sign-compare -Wno-old-style-declaration
 endif
 ECHO_CC = $(ECHO)
 CL := $(CC)
 PIE := $(shell $(CC) -dM -E - < /dev/null | grep pie)
 ifneq ($(PIE),)
  PIE := -no-pie
 endif
 CAST := $(shell $(CC) --help=warnings | grep cast-function-type)
 ifneq ($(CAST),)
  CAST := -Wno-cast-function-type
 endif
endif


######################## CPSS #####################################
###                                                             ###
###  Create list of include paths, modules to build and CFLAGS  ###
###  according to provided make flags                           ###
###                                                             ###
###################################################################

CFLAGS += -DMCD_RUN_WITH_CPSS
CFLAGS += -DIMPL_PP -DGT_PCI -D_linux

ifeq ($(WIDTH), 64)
 CFLAGS += -D__WORDSIZE=64 -DIS_64BIT_OS
endif

ifneq ($(CONFIG_GM),)

 ifeq ($(OS),Windows_NT)
    GM_LIB ?= \\\\fileril103\dev\Objects\cpss\bin\lib\simulation\libs\GM\VC10\PpInterfaceLibs\$(GM)\PPInterface.lib
 else
  GM_LIB ?= $(shell find /swdev/fileril103/Objects/cpss/bin/lib/simulation/libs/GM/Linux/PpInterfaceLibs/$(GM) -name "lib*.a")
 endif

endif

CPSS_INC_PATH := common/h
CPSS_INC_PATH += common/h/cpssAppPlatform
CPSS_INC_PATH += common/h/cpss/extServices/os
CPSS_INC_PATH += common/h/cpss/common/labServices/port/gop/port
CPSS_INC_PATH += common/src/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl
CPSS_INC_PATH += common/src/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/marvell/sd28firmware
CPSS_INC_PATH += common/src/cpss/common/labServices/port/gop/src/port/serdes/avago/aapl/include
CPSS_INC_PATH += mainExtUtils/h
CPSS_INC_PATH += cpssEnabler/utfWrappers/h
CPSS_INC_PATH += cpssEnabler/demo/h
CPSS_INC_PATH += cpssEnabler/mainOs/h
CPSS_INC_PATH += cpssEnabler/mainSysConfig/h
CPSS_INC_PATH += cpssEnabler/mainExtDrv/h
CPSS_INC_PATH += cpssEnabler/mainExtMac/h
CPSS_INC_PATH += cpssEnabler/mainPhy
CPSS_INC_PATH += cpssEnabler/mainCmd/h
CPSS_INC_PATH += cpssEnabler/mainCmd/src
CPSS_INC_PATH += cpssEnabler/mainSysConfig/src/appDemo/phy
CPSS_INC_PATH += mainExtUtils/src/extUtils/luaCLI/lua-5.1/src
CPSS_INC_PATH += shlibTestClients/h
CPSS_INC_PATH += luaCLI_standalone/src/FS
CPSS_INC_PATH += cpssEnabler/cpssAppPlatform/h
CPSS_INC_PATH += cpssEnabler/ezMpdCommon/h
CPSS_INC_PATH += cpssEnabler/cpssAppUtils/h

APPDEMO_MODULES := \
  cpssEnabler    \
  mainExtUtils

CPSS_MODULES := common

###  Optional packages and corresponding CFLAGS  ###

ifneq (,$(findstring DX,$(FAMILY)))
 CPSS_MODULES += mainPpDrv
 CFLAGS += -DCHX_FAMILY
 CPSS_INC_PATH += mainPpDrv/h
 CPSS_INC_PATH += mainPpDrv/src/cpss/generic/labServices/port/gop/h
 CPSS_INC_PATH += mainPpDrv/src/cpss/generic/labServices/port/gop/h/port
 CPSS_INC_PATH += mainPpDrv/src/cpss/generic/labServices/ddr/ddr3libv2/h/Driver
 CPSS_INC_PATH += mainPpDrv/src/cpss/generic/labServices/ddr/ddr3libv2/h/Driver/ddr3
 CPSS_INC_PATH += mainPpDrv/src/cpss/generic/labServices/ddr/ddr3libv2/h/Silicon
 CPSS_INC_PATH += mainPpDrv/src/cpss/generic/labServices/ddr/bap/h
 CPSS_INC_PATH += mainPpDrv/src/cpss/dxCh/dxChxGen/config/config88e1690/driver/include
 CPSS_INC_PATH += mainPpDrv/src/cpss/dxCh/dxChxGen/config/config88e1690/driver/include/h/msApi
 CPSS_INC_PATH += mainPpDrv/src/cpss/dxCh/dxChxGen/config/config88e1690/driver/include/h/platform
 CPSS_INC_PATH += mainPpDrv/src/cpss/dxCh/dxChxGen/config/config88e1690/driver/include/h/driver
 CPSS_INC_PATH += mainPpDrv/src/cpss/dxCh/dxChxGen/macSec/driver

 ifeq (,$(BUILD_CPSS_APP_PLAT))
  ifneq ($(OS),Windows_NT)
   INCLUDE_EXAMPLES := $(shell if [ -d referenceCode/examples ]; then echo Y; fi)
  else
   INCLUDE_EXAMPLES := $(shell if exist referenceCode\examples (echo Y))
  endif
 endif
endif
ifneq (,$(findstring PX,$(FAMILY)))
 CPSS_MODULES += mainPxDrv
 CFLAGS += -DPX_FAMILY -DGT_SMI
 CPSS_INC_PATH += mainPxDrv/h
 ifneq ($(OS),Windows_NT)
  INCLUDE_REFERENCE_HAL := $(shell if [ -d referenceCode/referenceHal ]; then echo Y; fi)
 else
  INCLUDE_REFERENCE_HAL := $(shell if exist referenceCode\referenceHal (echo Y))
 endif
 ifeq ($(INCLUDE_REFERENCE_HAL), Y)
  CPSS_INC_PATH += referenceCode/referenceHal/h referenceCode/referenceHal/px/bpe/h
  CFLAGS += -DREFERENCEHAL_BPE_EXISTS
 endif
endif

ifneq ($(INCLUDE_EXAMPLES)$(INCLUDE_REFERENCE_HAL),)
 APPDEMO_MODULES += referenceCode
endif

ifeq ($(CONFIG_INCLUDE_UTF), y)
 APPDEMO_MODULES += mainUT
 INCLUDE_ENHANCED_UTF := y
 CPSS_INC_PATH += mainUT/utfTool/h
 CPSS_INC_PATH += mainUT/mainPpDrv/h
 CPSS_INC_PATH += mainUT/utfTraffic/h
 CPSS_INC_PATH += mainUT/mainPxDrv/h
endif

ifneq ($(EXT_MODULE),)
 APPDEMO_MODULES += extension #Add extension directory for non-empty 'EXT_MODULE'
endif

ifeq ($(CONFIG_INCLUDE_GALTIS), y)
 APPDEMO_MODULES += mainGaltisWrapper
 CPSS_INC_PATH += mainGaltisWrapper/h
else
 EXCLUDE_GALTIS := EXCLUDE_LIB
 # Needed for TxRx
 CPSS_INC_PATH += mainGaltisWrapper/h
endif

ifeq ($(CONFIG_CMD_LUA_CLI), y)
 APPDEMO_MODULES += mainLuaWrapper
 CPSS_INC_PATH += mainLuaWrapper/h
 ifneq ($(EXT_MODULE),)
  CFLAGS += -DCMDFS_RAM_EMBEDDED_DATA_EXT_XML_EXT
 endif
 ifeq ($(CONFIG_INCLUDE_GALTIS), y)
  ifneq ($(OS),Windows_NT)
   INCLUDE_EMBEDDED_CMD := $(shell if [ -d embeddedCommands ]; then echo Y; else echo N; fi)
  else
   INCLUDE_EMBEDDED_CMD := $(shell if exist embeddedCommands (echo Y))
  endif
  ifeq ($(INCLUDE_EMBEDDED_CMD), Y)
   APPDEMO_MODULES += embeddedCommands
   CPSS_INC_PATH += embeddedCommands/h embeddedCommands/dx/dxEC/h embeddedCommands/px/pxEC/h
   CFLAGS += -DEMBEDDED_COMMANDS_EXISTS
  endif
 endif
endif
ifeq (1, $(simFilesUsed))
 CPSS_MODULES += simulation
 CFLAGS += -DASIC_SIMULATION
 SIM := Sim
 CPSS_INC_PATH += simulation/simGeneral/h
 CPSS_INC_PATH += simulation/simDevices/h
 CPSS_INC_PATH += simulation/simOs/h
 CPSS_INC_PATH += simulation/simDevices/src/asicSimulation/SKernel/sEmbeddedCpu/firmware/pipe_Pha/inc
 CPSS_INC_PATH += simulation/simGeneral/h/common/SHOST
 CPSS_INC_PATH += simulation/simDevices/src/asicSimulation/SKernel/sEmbeddedCpu/cm3/h
 SHLIB_LDFLAGS := -Xlinker --wrap -Xlinker calloc -Xlinker --wrap -Xlinker free -Xlinker --wrap -Xlinker malloc \
                  -Xlinker --wrap -Xlinker realloc
else
 LINUX_NOKM ?= YES
endif

##
ifeq ($(CONFIG_INCLUDE_EZ_BRINGUP), y)
 CPSS_INC_PATH += cpssEnabler/ez_bringup/pdl/h
 CPSS_INC_PATH += cpssEnabler/ez_bringup/iDbgPdl/h

endif

ifeq ($(CONFIG_INCLUDE_MPD), y)
 CPSS_INC_PATH += cpssEnabler/mainPhy/MPD/h
 CPSS_INC_PATH += cpssEnabler/mainPhy/MPD/external_phy/mtd/src
 CPSS_INC_PATH += cpssEnabler/mainPhy/MPD/external_phy/alaska_c/src
 ifeq (y, $(CONFIG_ASIC_SIMULATION))
  CFLAGS += -DPHY_SIMULATION
 endif
endif
##
ifeq ($(CONFIG_INCLUDE_TM), y)
 CPSS_MODULES-$(CONFIG_INCLUDE_TM) += mainTmDrv
 CPSS_INC_PATH += mainTmDrv/h
 CPSS_INC_PATH += mainTmDrv/h/private
 CPSS_INC_PATH += mainTmDrv/h/core
 CPSS_INC_PATH += mainTmDrv/h/platform
endif
ifeq ($(OS_RUN),linux)
 CFLAGS += -DLINUX -DV2L_PTHREADS
endif
ifeq ($(LINUX_NOKM), YES)
 CFLAGS += -DLINUX_NOKM -DNOKM_DRV_EMULATE_INTERRUPTS
endif
ifneq (, $(filter $(STACK_SIZE_LIMIT), Y R))
ifneq ($(shell gcc --help=common|grep stack-usage|wc -l), 0)
CFLAGS += -fstack-usage
STACK_SIZE_LIMIT_REPORT := 4096
else
override STACK_SIZE_LIMIT := N
endif
else
endif

ifeq ($(CONFIG_SHARED_MEMORY), y)
 LUA_MODULES := luaCLI_standalone
 SHARED_CFLAGS := -fPIC -DPIC -D$(SHARED_LIB_LD_ARCH)
 LUACLI_CFLAGS := -DENV_POSIX -DCLI_STANDALONE
 CPSS_SPLIT_LIBRARIES = 1

 SHLIB_LDFLAGS := -shared $(SHLIB_LDFLAGS) $(TARGET_SPECIFIC_CFLAGS)
 LDFLAGS := -Xlinker -z -Xlinker nocopyreloc -rdynamic -lpthread -lrt -ldl -lm $(TARGET_SPECIFIC_CFLAGS)
else
 LDFLAGS := -lrt -lpthread -lm -ldl -rdynamic $(LDFLAGS-y) $(TARGET_SPECIFIC_CFLAGS)
endif

ifeq (LIB,$(BUILD_CPSS_APP_PLAT))
#CAP requires separate libs for os and extdrv - which requires the below flag to be turned on
CPSS_SPLIT_LIBRARIES = 1
endif

CPSS_MODULES += $(CPSS_MODULES-y)

################### Create list of sources ########################
###                                                             ###
###  Recursively dive into source folder tree and return with   ###
###  lists of source *.c files and updated CFLAGS               ###
###                                                             ###
###################################################################
ifneq ("$(MAKECMDGOALS)", "clean")
 ifneq ("$(MAKECMDGOALS)", "cpssLib")
  include $(patsubst %,%/$(MAKEFILE),$(CPSS_MODULES) $(APPDEMO_MODULES) $(LUA_MODULES))
 else
  include $(patsubst %,%/$(MAKEFILE),$(CPSS_MODULES) $(LUA_MODULES))
 endif
endif
ifeq ($(CONFIG_SHARED_MEMORY), y)
 APPDEMO_MODULES := $(filter-out cpssEnabler , $(APPDEMO_MODULES)) # In shared lib mode cpssEnabler is split to sub modules
endif

# CFLAGS may be updated by recursive Makefiles, so this is a good location to gather it all up
ifeq ($(TOOL_TYPE), vc)
CFLAGS := $(TARGET_SPECIFIC_CFLAGS) $(addprefix -I, $(CPSS_INC_PATH)) $(CFLAGS) $(CONFIG_CFLAGS-y)
VC_CFLAGS := -nologo -Ot -Zi -Fd"$(BUILD_FOLDER)/"
ifneq ($(CONFIG_GM),)
  VC_CFLAGS += -MTd
endif
else
CFLAGS := -Wall -Wextra -Werror -Wno-error=cpp $(CAST) -ansi $(TARGET_SPECIFIC_CFLAGS) -fno-builtin -funroll-loops \
           $(SHARED_CFLAGS) $(addprefix -I, $(CPSS_INC_PATH)) $(CFLAGS) $(CONFIG_CFLAGS-y)
#-Wno-cast-function-type  -Wextra
endif

################# Create list of objects and libs #################
###                                                             ###
###################################################################

# List of source file packages, based on modules
# Should look like:  CPSSENABLER_SRC   MAINPPDRV_SRC   MAINEXTUTILS_SRC ...
SRC_PACKAGES := $(addsuffix _SRC,$(call uppercase,$(CPSS_MODULES) $(APPDEMO_MODULES)))

ifeq (LIB,$(BUILD_CPSS_APP_PLAT))
CPSS_MODULES += mainExtUtils \
                mainLuaWrapper

LIB_CPSS_ENABLER_MODULES := cpssAppRef mainCmd demo utfWrappers gtOs gtStack gtUtil mainExtDrv extPhyM
CPSS_OBJ_LIBS := $(foreach M,$(CPSS_MODULES),$(BUILD_FOLDER)/$(M)/libs/$(call uppercase,$(M)).a)
CPSS_ENABLER_OBJ_LIBS := $(foreach M,$(LIB_CPSS_ENABLER_MODULES),$(BUILD_FOLDER)/cpssEnabler/libs/$(call uppercase,$(M)).a)
OBJ_LIBS := $(CPSS_OBJ_LIBS) $(CPSS_ENABLER_OBJ_LIBS)

# List of objects per library
SRC_PACKAGES_SUB_MODULES := $(addprefix cpssEnabler_, $(addsuffix _SRC1, $(LIB_CPSS_ENABLER_MODULES) ))
$(foreach S,$(SRC_PACKAGES_SUB_MODULES),$(eval $(call uppercase, $(patsubst cpssEnabler_%_SRC1,%_OBJ,$(S))) := $(patsubst %.c,$(BUILD_FOLDER)/%.o,$($(S)))))
else
# List of object libraries, full path. Each library corresponds to a source package
# Should look like: $(BUILD_FOLDER)/cpssEnabler/libs/CPSSENABLER.a $(BUILD_FOLDER)/mainPpDrv/libs/MAINPPDRV.a  ...
CPSS_OBJ_LIBS := $(foreach M,$(CPSS_MODULES),$(BUILD_FOLDER)/$(M)/libs/$(call uppercase,$(M)).a)
APPDEMO_OBJ_LIBS := $(foreach M,$(APPDEMO_MODULES),$(BUILD_FOLDER)/$(M)/libs/$(call uppercase,$(M)).a)
OBJ_LIBS := $(CPSS_OBJ_LIBS) $(APPDEMO_OBJ_LIBS)
endif

# Create list of objects per object library, based on corresponding SRC package.
# This evaluates to something like:
# COMMON_OBJ :=  $(BUILD_FOLDER)/common/.../cpssHwInit.o  $(BUILD_FOLDER)/common/.../prvCpssHwMultiPortGroups.o  ...
# MAINPPDRV_OBJ :=  $(BUILD_FOLDER)/mainPpDrv/.../prvCpssGenBrgFdbIsrSig.o  $(BUILD_FOLDER)/mainPpDrv/.../prvCpssGenBrgLog.o  ...
$(foreach S,$(SRC_PACKAGES),$(eval $(patsubst %_SRC,%_OBJ,$(S)) := $(patsubst %.c,$(BUILD_FOLDER)/%.o,$($(S)))))

# Same for shared memory sub modules
ifeq ($(CONFIG_SHARED_MEMORY), y)
 LIB_CPSS_MODULES := mainExtDrv shrMemIntraCPSSdata # For libcpss.a
 LIB_HELPER_MODULES := gtOs gtStack gtUtil mainExtDrvShared # For libhelper.a
 APPDEMO_SHARED_MODULES := utfWrappers demo confi extPhyM luaCLI mainCmd mainPhy mainSysConfig cpssAppRef# dragonite
 LIB_CPSS_OBJ_LIBS := $(foreach M,$(LIB_CPSS_MODULES),$(BUILD_FOLDER)/cpssEnabler/libs/$(call uppercase,$(M)).a)
 LIB_HELPER_OBJ_LIBS := $(foreach M,$(LIB_HELPER_MODULES),$(BUILD_FOLDER)/cpssEnabler/libs/$(call uppercase,$(M)).a)
 APPDEMO_SHARED_OBJ_LIBS := $(foreach M,$(APPDEMO_SHARED_MODULES),$(BUILD_FOLDER)/cpssEnabler/libs/$(call uppercase,$(M)).a)

 # List of objects per library
 SRC_PACKAGES_SUB_MODULES := $(addprefix cpssEnabler_, $(addsuffix _SRC1, $(LIB_CPSS_MODULES) $(LIB_HELPER_MODULES) $(APPDEMO_SHARED_MODULES)))
 $(foreach S,$(SRC_PACKAGES_SUB_MODULES),$(eval $(call uppercase, $(patsubst cpssEnabler_%_SRC1,%_OBJ,$(S))) := $(patsubst %.c,$(BUILD_FOLDER)/%.o,$($(S)))))

 LUACLI_STANDALONE_OBJ := $(patsubst %.c, $(BUILD_FOLDER)/%.o, $(LUACLI_STANDALONE_SRC))
 LUACLI_STANDALONE_OBJ += $(patsubst %.c, $(BUILD_FOLDER)/luaCLI_standalone/%.o, $(MAINLUAWRAPPER_LUA_STANDALONE_SRC))

 SHARED_MEMORY_REF_APPS := $(BUILD_FOLDER)/luaCLI
 ifneq (,$(findstring DX,$(FAMILY)))
  SHARED_MEMORY_REF_APPS += $(BUILD_FOLDER)/RxTxProcess $(BUILD_FOLDER)/fdbLearning
 endif
ifeq ($(CONFIG_ASIC_SIMULATION),y)
SHARED_MEMORY_REF_APPS += $(BUILD_FOLDER)/simStandAlone
endif
endif

# Full list of all source files to be compiled
SRC := $(foreach S, $(SRC_PACKAGES) $(SRC_PACKAGES_SUB_MODULES), $($(S)))

# List of all object and dependency files
OBJ := $(patsubst %.c,$(BUILD_FOLDER)/%.o,$(SRC))
ifneq (DEP_NO, $(DEPENDENCY))
DEP += $(patsubst %.o,%.d,$(OBJ))
endif

# Create dependency of each lib.a in corresponding LIB_OBJ list
$(foreach lib,$(OBJ_LIBS),$(eval $(lib): $($(notdir $(subst .A,,$(call uppercase,$(lib))))_OBJ)))
$(foreach lib,$(LIB_CPSS_OBJ_LIBS),$(eval $(lib): $($(notdir $(subst .A,,$(call uppercase,$(lib))))_OBJ)))
$(foreach lib,$(LIB_HELPER_OBJ_LIBS),$(eval $(lib): $($(notdir $(subst .A,,$(call uppercase,$(lib))))_OBJ)))
$(foreach lib,$(APPDEMO_SHARED_OBJ_LIBS),$(eval $(lib): $($(notdir $(subst .A,,$(call uppercase,$(lib))))_OBJ)))
$(BUILD_FOLDER)/luaCLI_standalone/libs/LUACLI_STANDALONE.a: $(LUACLI_STANDALONE_OBJ)

############################ Targets ##############################
###                                                             ###
###################################################################

.PHONY: all

$(BUILD_FOLDER):
	$(MKDIR) $@

all: install kernel
clean_all: clean_cpss clean_rootfs clean_kernel clean_modules
	rm -f $(BUILD_FOLDER)/*.img

.PHONY: kernel

$(CONFIG):
ifeq ($(OS), Windows_NT)
	@ echo $(subst ',,$(subst ",,$(config))) > tmp.bat
	@ sed -i -e 's/;\s\+/>>$(CONFIG)\n/g' tmp.bat
else
	@ echo $(config) > tmp.bat
	@ sed -i -e 's/;\s*/>>$(CONFIG)\n/g' tmp.bat
endif
	@ rm -f $(CONFIG) 
	@ $(SOURCE) tmp.bat
	@ rm -f tmp.bat


help:
	@echo
	@echo "CPSS build system"
	@echo "================="
	@echo
	@echo "Usage: make TARGET=<target> FAMILY=<DX / PX / DXPX> [CROSS_COMPILE=<toolchain>] <make_goal>"
	@echo
	@echo "Optional flags for $(ENABLER) build (and default value):"
	@echo "   [DEBUG_INFO=n] [INCLUDE_UTF=n] [INCLUDE_GALTIS=n] [SHARED_MEMORY=n] [INCLUDE_TM=n]"
	@echo "   [CMD_LUA_CLI=y] [LOG_ENABLE=y] [INCLUDE_EZ_BRINGUP=n] [INCLUDE_MPD=n]"
	@echo "   [API_LOCK_PROTECTION=y] [CPSS_USE_MUTEX_PROFILER=n] [GM=bobcat3 / aldrin2 / falcon]"
	@echo "   [TRAFFIC_API_LOCK_DISABLE=n] [DEPENDENCY=DEP_NO] [STACK_SIZE_LIMIT=n/[y/r]]"
	@echo " "
	@echo "Optional flags for kernel/modules build:"
	@echo "   [KERNEL_FOLDER=]"
	@echo " "
	@echo "target" represents file name from cpss/tools/build/targets:
	@echo $(targets)
	@echo " "
	@echo "List of make goals:"
	@sh -c "make -p no_targets__ -f Makefile : 2>/dev/null | awk -F':' '/^[a-zA-Z0-9][^\$$#\/\\t=]*:([^=]|$$)/ {split(\$$1,A,/ /);for(i in A)print A[i]}' | grep -v '__\$$|Makefile|make' | sort"
	@echo


ifeq ($(OS_RUN), win32)

# Check if using VC10 or VC10_64bit to choose correct shost / slan library files
ifeq ($(Platform), X64)
LIB_ARCH := _64bit
endif

# Change name of libs from lib.a to lib.lib
OBJ_LIBS := $(patsubst %.a, %.lib, $(OBJ_LIBS))

# Create dependency of each lib.lib in corresponding LIB_OBJ list
$(foreach lib,$(OBJ_LIBS),$(eval $(lib): $($(notdir $(subst .LIB,,$(call uppercase,$(lib))))_OBJ)))

ifeq ($(TOOL_TYPE), vc)

ifeq ($(OS), Windows_NT)
FILER_LIB_PATH := //fileril103/dev/objects/cpss/bin/lib/simulation/libs
SIM_LIB_PATH := simulation/extern_lib

$(BUILD_FOLDER)/simulation/libs/shost/SHOST.lib:
	@ if not exist $(dir $@) ($(MKDIR) $(subst /,\,$(dir $@)))
	@ if exist $(subst /,\,$(FILER_LIB_PATH)\shost\VC10$(LIB_ARCH)\SHOST.lib) (cp $(FILER_LIB_PATH)/shost/VC10$(LIB_ARCH)/SHOST.lib $@) \
		else (if exist $(subst /,\,$(SIM_LIB_PATH)\shost\VC10$(LIB_ARCH)\SHOST.lib) (cp $(SIM_LIB_PATH)/shost/VC10$(LIB_ARCH)/SHOST.lib $@) \
		else (echo *** Error: cannot find SHOST.lib))

$(BUILD_FOLDER)/simulation/libs/slan/SLAN.lib:
	@ if not exist $(dir $@) ($(MKDIR) $(subst /,\,$(dir $@)))
	@ if exist $(subst /,\,$(FILER_LIB_PATH)\slan\VC10$(LIB_ARCH)\SLAN.lib) (cp $(FILER_LIB_PATH)/slan/VC10$(LIB_ARCH)/SLAN.lib $@) \
		else (if exist $(subst /,\,$(SIM_LIB_PATH)\slan\VC10$(LIB_ARCH)\SLAN.lib) (cp $(SIM_LIB_PATH)/slan/VC10$(LIB_ARCH)/SLAN.lib $@) \
		else (echo *** Error: cannot find SLAN.lib))

%.lib:
	@ $(ECHO) AR $@
	@ if not exist $(dir $@) ($(MKDIR) $(subst /,\,$(dir $@)))
	@ echo "-nologo" -ignore:4006 > $@.list
	@ for %%i in ($^) do @echo %%i >> $@.list
	@ echo "-out:$@" >> $@.list
	@ $(AR) @$@.list
#	echo "dumpbin -symbols $@ > $(dir $@)$(notdir $@).list"

else

#
# Target copy for SLAN and SHOST on Linux Host
#

%.lib:
	@ $(ECHO) AR $@
	$(MKDIR) $(dir $@)
	@ $(AR) /NOLOGO /IGNORE:4006 /OUT:$@ $^
endif

else
# Tool Type GCC
%.lib:
	@ $(ECHO) AR $@
	$(MKDIR) $(dir $@)
	@ $(AR) crs $@ $^

endif

clean:
	rm -rf $(BUILD_FOLDER)

else # Linux GNU

%.a:
	@ $(ECHO) AR $@
	$(MKDIR) $(dir $@)
	$(AR) crus $@ $^

clean:
	@ if [[ $(BUILD_FOLDER) != / ]]; then rm -rf $(BUILD_FOLDER); fi
	@ rm $(CONFIG)
ifneq ($(IS_FREERTOS),)
	@ ./extension/srvCpu/firmware/internal/build_free_rtos.sh clean
endif

endif

.PHONY: $(BUILD_FOLDER)/version_info.c
$(BUILD_FOLDER)/version_info.c:
	echo const char *VERSION_DATE = '"$(VERSION_DATE)"'';'  >  $@
	echo const char *VERSION_FLAGS = '"$(VERSION_FLAGS)"'';'  >> $@


ifeq (1, $(simFilesUsed))

 #   Simulation
 ######################
 # generate map file
 # 1. Generate list
 # 2. Create .c file
ifeq ($(OS), Windows_NT)
symtable.c: $(OBJ) $(OBJ_LIBS)
	echo " " > $(BUILD_FOLDER)/symtable.list
	for %%l in ($(OBJ_LIBS)); do dumpbin -nologo -symbols %%l >> $(BUILD_FOLDER)/symtable.list
	awk -f tools/bin/vc_mksymtbl.awk $(BUILD_FOLDER)/symtable.list > symtable.c

clean_exe:
	rm -f $(BUILD_FOLDER)/$(ENABLER).*

$(ENABLER).exe: clean_exe $(BUILD_FOLDER)/symtable.o $(BUILD_FOLDER)/simulation/libs/shost/SHOST.lib $(BUILD_FOLDER)/simulation/libs/slan/SLAN.lib
	@ $(ECHO) LD $@
	$(LD) -nologo -opt:noref -subsystem:console $(OBJ_LIBS) $(filter-out clean_exe,$^) $(GM_LIB) \
	User32.lib WinMM.Lib OLDNAMES.lib WSock32.Lib Shlwapi.lib \
	-LARGEADDRESSAWARE -NODEFAULTLIB:LIBCMT \
	-out:$(BUILD_FOLDER)/$@ -DEBUG -map:$(BUILD_FOLDER)/$(@:.exe=.map)
	@ $(ECHO) EXE file : $(subst /,\,$(BUILD_FOLDER))\$@
#libucrt.lib mincore.lib
cpsslibs: $(OBJ) $(OBJ_LIBS)
	@ $(ECHO) libraries are ready:
	@ for %%l in ($(OBJ_LIBS)); do $(ECHO) %%l
else
#    On a Linux Host
########################
symtable.c: $(OBJ) $(OBJ_LIBS)
	echo " " > $(BUILD_FOLDER)/symtable.list
	for lib in $(OBJ_LIBS); do nm -g $$lib >> $(BUILD_FOLDER)/symtable.list; done
	awk -f tools/bin/gcc_mksymtbl.awk $(BUILD_FOLDER)/symtable.list > symtable.c

$(ENABLER).exe: $(BUILD_FOLDER)/symtable.o $(BUILD_FOLDER)/simulation/libs/shost/SHOST.lib $(BUILD_FOLDER)/simulation/libs/slan/SLAN.lib
	@ $(ECHO) LD $@
	$(LD) -Wl,--start-group $(OBJ_LIBS) -Wl,--end-group $^ $(GM_LIB) \
	-luser32  -lwinmm -lwsock32 -lshlwapi \
	-o $(BUILD_FOLDER)/$@
#libucrt.lib mincore.lib

cpsslibs: $(OBJ) $(OBJ_LIBS)
	@ $(ECHO) libraries are ready:
	@ $(ECHO) $(OBJ_LIBS) | fmt -w 0
endif

else
 #   BM
 ######################
 $(BUILD_FOLDER)/symtable.c: $(OBJ) $(OBJ_LIBS)

 #   FW files:
 ######################
 #   must check dependancy after any git snapshot change
 #   Create dummy file with date = last reflog in git.
 #   All FW files depend on it, so if any is older, build script is executed.
 ifneq ($(IS_FREERTOS),)
  FW_FILES := $(patsubst %,common/src/cpss/common/srvCpu/mvHwsServiceCpu%Fw.h,Bobk Bc2 Cm3BobK Cm3Aldrin Cm3Bc3 Cm3Pipe Cm3Aldrin2 Cm3Raven)
  # generate git date reference file
  REFFILE := $(BUILD_FOLDER)/.git_ref_date
  REFDATE := $(shell git reflog -1 --date=iso 2>/dev/null | sed -n 's/^.*{\([^}]*\)}.*$$/\1/p')
  $(shell mkdir -p $(BUILD_FOLDER))
  $(shell touch -d "$(REFDATE)" $(REFFILE))

  # If build of FWs necessary, create file to lock the operation for a single thread
  # Otherwise, in case of -j n, the build script will be started multiple times
$(FW_FILES): $(REFFILE)
#	@ $(ECHO) Building FWs:
	@ flock $(REFFILE).lock -c "if [ -e $(REFFILE) ]; then rm $(REFFILE)*; ./extension/srvCpu/firmware/internal/build_free_rtos.sh; fi"

$(BUILD_FOLDER)/common/src/cpss/common/srvCpu/prvSrvCpuFirmware.o: $(FW_FILES)
 endif

endif
ifeq ($(CONFIG_SHARED_MEMORY), y)
$(BUILD_FOLDER)/luaCLI_standalone/%.o: luaCLI_standalone/%.c
	$(call compile, $(filter-out -DENV_MAINCMD,$(CFLAGS)) $(LUACLI_CFLAGS), $(BUILD_FOLDER)/luaCLI_standalone)

$(BUILD_FOLDER)/luaCLI_standalone/%.o: %.c
	$(call compile, $(filter-out -DENV_MAINCMD,$(CFLAGS)) $(LUACLI_CFLAGS), $(BUILD_FOLDER)/luaCLI_standalone)

$(ENABLER): $(FW_FILES) $(BUILD_FOLDER)/libcpss.so $(BUILD_FOLDER)/libhelper.so $(APPDEMO_OBJ_LIBS) $(APPDEMO_SHARED_OBJ_LIBS) $(BUILD_FOLDER)/version_info.o $(SHARED_MEMORY_REF_APPS)
	@ $(ECHO) LD $@
	$(LD) -Xlinker --whole-archive $(filter %.a %.o, $^) -Xlinker --no-whole-archive \
	$(filter %.so, $^) $(LDFLAGS) -Xlinker -Map -Xlinker $(BUILD_FOLDER)/$@.map -o $(BUILD_FOLDER)/$@
ifeq ($(STACK_SIZE_LIMIT), R)
	@ $(ECHO) "Function with stack size larger than $(STACK_SIZE_LIMIT_REPORT)"
	$(AWK) '$$2 > $(STACK_SIZE_LIMIT_REPORT)' \
		$(shell find $(BUILD_FOLDER)/ -name "*.su")|\
		grep -e "static" -e "dynamic,bounded" || /bin/true
endif
else

$(OBJ): $(CONFIG)

cpssLib: $(CPSS_OBJ_LIBS)
	@ $(ECHO) libraries are ready:
	@ $(ECHO) $^ | fmt -w 0

cpssLibExt: $(OBJ_LIBS)
	@ $(ECHO) libraries are ready:
	@ $(ECHO) $^ | fmt -w 0


# Static link appDemo
ifeq (LIB,$(BUILD_CPSS_APP_PLAT))
$(ENABLER): $(BUILD_FOLDER) $(OBJ) $(OBJ_LIBS) $(BUILD_FOLDER)/version_info.o
else
$(ENABLER): $(BUILD_FOLDER) $(OBJ) $(OBJ_LIBS) $(BUILD_FOLDER)/version_info.o
	@ $(ECHO) LD $@
	$(LD) -Wl,--whole-archive $(OBJ_LIBS) -Wl,--no-whole-archive $(BUILD_FOLDER)/version_info.o $(GM_LIB) \
	$(LDFLAGS) -Xlinker -Map -Xlinker $(BUILD_FOLDER)/$@.map -o $(BUILD_FOLDER)/$@
# Produce stack limit exceed report
ifeq ($(STACK_SIZE_LIMIT), R)
	@ $(ECHO) "Function with stack size larger than $(STACK_SIZE_LIMIT_REPORT)"
	$(AWK) '$$2 > $(STACK_SIZE_LIMIT_REPORT)' \
		$(shell find $(BUILD_FOLDER)/ -name "*.su")|\
		grep -e "static" -e "dynamic,bounded" || /bin/true
endif
endif
ifeq ($(CONFIG_DEBUG_INFO), y)
# create stripped binary for target
	$(STRIP) $(BUILD_FOLDER)/$@ -o $(BUILD_FOLDER)/$@_stripped
endif
endif

$(BUILD_FOLDER)/%.o: %.c
	$(call compile, $(CFLAGS), $(BUILD_FOLDER))

# debug target. usage: make print-var will print value of variable "var"
print-%  : ; @echo $* = $($*)

ifneq (DEP_NO, $(DEPENDENCY))
-include $(DEP)
endif


# related to libhelper.so and libcpss.so

SCRIPTLD_ROOT := cpssEnabler/mainOs/src/gtOs/sharedMemory/scriptsld/linux$(SIM)
SCRIPTLD_CPSS_LIB := $(SCRIPTLD_ROOT)/libcpss_ld_script.$(SHARED_LIB_LD_ARCH)
SCRIPTLD_HELPER_LIB := $(SCRIPTLD_ROOT)/libhelper_ld_script.$(SHARED_LIB_LD_ARCH)

# PIC = position independent code
ifeq ("Y", "$(PIC)")
$(BUILD_FOLDER)/libcpss.so: $(CPSS_OBJ_LIBS) $(LIB_CPSS_OBJ_LIBS)
	@ $(ECHO) LD $@
	$(LD) $(SHLIB_LDFLAGS) -Xlinker --whole-archive $^ -Xlinker --no-whole-archive \
	-Xlinker -Map -Xlinker $(@:.so=.map)  \
	-Xlinker -soname -Xlinker $(@F) -o $@

$(BUILD_FOLDER)/libhelper.so: $(LIB_HELPER_OBJ_LIBS)
	@ $(ECHO) LD $@
	$(LD) $(SHLIB_LDFLAGS) -Xlinker --whole-archive $^ -Xlinker --no-whole-archive \
	-Xlinker -Map -Xlinker $(@:.so=.map)  \
	-Xlinker -soname -Xlinker $(@F) -o $@
else
$(BUILD_FOLDER)/libcpss.so: $(CPSS_OBJ_LIBS) $(LIB_CPSS_OBJ_LIBS)
	@ $(ECHO) LD $@
	$(LD) $(SHLIB_LDFLAGS) -Xlinker --whole-archive $^ -Xlinker --no-whole-archive \
	-Xlinker -Map -Xlinker $(@:.so=.map) -T $(SCRIPTLD_CPSS_LIB) \
	-Xlinker -soname -Xlinker $(@F) -o $@

$(BUILD_FOLDER)/libhelper.so: $(LIB_HELPER_OBJ_LIBS)
	@ $(ECHO) LD $@
	$(LD) $(SHLIB_LDFLAGS) -Xlinker --whole-archive $^ -Xlinker --no-whole-archive \
	-Xlinker -Map -Xlinker $(@:.so=.map)  -T $(SCRIPTLD_HELPER_LIB) \
	-Xlinker -soname -Xlinker $(@F) -o $@
endif

$(BUILD_FOLDER)/RxTxProcess: $(BUILD_FOLDER)/shlibTestClients/src/RxTxProcess.o $(BUILD_FOLDER)/mainExtUtils/src/extUtils/rxEventHandler/rxEventHandler.o  $(BUILD_FOLDER)/libcpss.so $(BUILD_FOLDER)/libhelper.so
	@ $(ECHO) LD $@
	$(LD) $^ $(LDFLAGS) -Xlinker -Map -Xlinker $@.map -o $@

$(BUILD_FOLDER)/fdbLearning: $(BUILD_FOLDER)/shlibTestClients/src/fdbLearning.o $(BUILD_FOLDER)/mainExtUtils/src/extUtils/auEventHandler/auEventHandler.o $(BUILD_FOLDER)/libcpss.so $(BUILD_FOLDER)/libhelper.so
	@ $(ECHO) LD $@
	$(LD) $^ $(LDFLAGS) -Xlinker -Map -Xlinker $@.map -o $@

$(BUILD_FOLDER)/luaCLI: $(BUILD_FOLDER)/libcpss.so $(BUILD_FOLDER)/libhelper.so $(BUILD_FOLDER)/luaCLI_standalone/libs/LUACLI_STANDALONE.a $(BUILD_FOLDER)/mainExtUtils/libs/MAINEXTUTILS.a
	@ $(ECHO) LD $@
	$(LD) -Xlinker --whole-archive $(filter %.a, $^) -Xlinker --no-whole-archive \
	$(filter %.so, $^) $(LDFLAGS) -Xlinker -Map -Xlinker $@.map -o $@
##############################SIM-STANDALONE########################################
# This is a minimal set of objects in order to build standalone simulation
####################################################################################

SIM_STANDALONE_OBJ := $(BUILD_FOLDER)/cpssEnabler/mainOs/src/gtOs/linux/osLinuxSem.o
SIM_STANDALONE_OBJ += $(BUILD_FOLDER)/cpssEnabler/mainOs/src/gtOs/linux/osLinuxIo.o
SIM_STANDALONE_OBJ += $(BUILD_FOLDER)/cpssEnabler/mainOs/src/gtOs/linux/pthreads/lsemLib.o
SIM_STANDALONE_OBJ += $(BUILD_FOLDER)/cpssEnabler/mainOs/src/gtOs/linux/pthreads/ltaskLib.o
SIM_STANDALONE_OBJ += $(BUILD_FOLDER)/cpssEnabler/mainOs/src/gtOs/linux/osLinuxTimer.o
SIM_STANDALONE_OBJ += $(BUILD_FOLDER)/cpssEnabler/mainOs/src/gtOs/linux/osObjIdLib.o
SIM_STANDALONE_OBJ += $(BUILD_FOLDER)/cpssEnabler/mainOs/src/gtOs/linux/pthreads/lkernelLib.o
SIM_STANDALONE_OBJ += $(BUILD_FOLDER)/cpssEnabler/mainOs/src/gtOs/linux/osGlobalDb.o
SIM_STANDALONE_OBJ += $(BUILD_FOLDER)/cpssEnabler/mainOs/src/gtOs/linux/osLinuxGen.o
SIM_STANDALONE_OBJ += $(BUILD_FOLDER)/cpssEnabler/mainOs/src/gtOs/init/osGlobalNonSharedEnablerInitVars.o
SIM_STANDALONE_OBJ += $(BUILD_FOLDER)/cpssEnabler/mainOs/src/gtOs/init/osGlobalSharedEnablerInitVars.o
SIM_STANDALONE_OBJ += $(BUILD_FOLDER)/cpssEnabler/mainOs/src/gtUtil/gtBmPool.o
SIM_STANDALONE_OBJ += $(BUILD_FOLDER)/cpssEnabler/mainCmd/src/cmdShell/os/linux/os_lxExecute.o
SIM_STANDALONE_OBJ += $(BUILD_FOLDER)/simStandalone/stubs.o
SIM_STANDALONE_OBJ += $(BUILD_FOLDER)/simStandalone/main.o
SIM_STANDALONE_OBJ += $(BUILD_FOLDER)/cpssEnabler/mainOs/src/gtOs/linux/pthreads/lmsgqLib.o
SIM_STANDALONE_OBJ += $(BUILD_FOLDER)/cpssEnabler/mainOs/src/gtStack/linux/osLinuxSocket.o
SIM_STANDALONE_OBJ += $(BUILD_FOLDER)/cpssEnabler/mainOs/src/gtStack/linux/osLinuxSelect.o
SIM_STANDALONE_OBJ += $(BUILD_FOLDER)/cpssEnabler/mainOs/src/gtOs/linux/linuxSim/stubs.o
SIM_STANDALONE_OBJ += $(BUILD_FOLDER)/simulation/libs/SIMULATION.a

$(BUILD_FOLDER)/simStandAlone: $(SIM_STANDALONE_OBJ)
	@ $(ECHO) LD $@
	$(LD) $^ $(LDFLAGS) -Xlinker -Map -Xlinker $@.map -o $@
######################## KERNEL ###############################
KERNEL_BOOT_PATH := $(KERNEL_FOLDER)/arch/$(ARCH)/boot
KERNEL_DTB_FILE := $(patsubst %.dts,%.dtb,$(DEVICE_TREE_FILE))
KERNEL_DTB_FILE_PATH := $(addprefix $(KERNEL_BOOT_PATH)/dts/, $(KERNEL_DTB_FILE))

clean_kernel:
ifneq ("$(TARGET)", "")
	@ $(MAKE) -C $(KERNEL_FOLDER) distclean
	@ rm $(KERNEL_BOOT_PATH)/dts/*.dtb -f
endif

$(KERNEL_FOLDER)/.config:
	@ echo Checking out commit $(LINUX_KERNEL_COMMIT)
	@ git -C $(KERNEL_FOLDER) checkout $(LINUX_KERNEL_COMMIT)
	@ echo Creating defconfig $(DEFCONFIG)
	@ $(MAKE) -C $(KERNEL_FOLDER) $(DEFCONFIG)
	@ $(ADDITIONAL_DEFCONFIG)

$(KERNEL_BOOT_PATH)/$(KERNEL_IMAGE_NAME): $(KERNEL_FOLDER)/.config $(KERNEL_DTB_FILE_PATH)
	@ echo Compiling $(KERNEL_IMAGE_NAME)
	@ touch $(KERNEL_FOLDER)/.scmversion
	@ $(MAKE) -C $(KERNEL_FOLDER) LOCALVERSION= $(KERNEL_IMAGE_NAME)

%.dtb: %.dts $(KERNEL_FOLDER)/.config
	@ $(MAKE) -C $(KERNEL_FOLDER) $(notdir $@)

kernel: $(KERNEL_BOOT_PATH)/$(KERNEL_IMAGE_NAME) $(KERNEL_DTB_FILE_PATH)
	@ echo $(KERNEL_FOLDER)/arch/$(ARCH)/boot/$(KERNEL_IMAGE_NAME) created successfully!

modules: $(KERNEL_FOLDER)/.config
	$(MAKE) -C $(KERNEL_FOLDER) LOCALVERSION= M=$(MODULES_FOLDER) clean
	$(MAKE) -C $(KERNEL_FOLDER) LOCALVERSION= M=$(MODULES_FOLDER) modules

clean_modules:
	$(MAKE) -C $(KERNEL_FOLDER) LOCALVERSION= M=$(MODULES_FOLDER) clean


######################## ROOTFS #############################

clean_rootfs:
	@ rm -rf $(BUILD_FOLDER)/rootfs $(BUILD_FOLDER)/rootfs_stamp

.PHONY: rootfs
rootfs:
	@ if [ ! -f $(BUILD_FOLDER)/rootfs_stamp ]; then \
		set -e; \
		tools/build/rootfs/crfs.sh $(BUILD_FOLDER)/rootfs $(CROSS_COMPILE) /nfs/pt/swdev/areas/readonly/swtools/devsources/root_fs/files/busybox/busybox-1.31.1.tar.bz2; \
		touch $(BUILD_FOLDER)/rootfs_stamp; \
		echo Created rootfs at $(BUILD_FOLDER)/rootfs; \
	else \
		echo rootfs already exists.; \
	fi; \
	echo



####################### install #############################

install: $(BUILD_FOLDER)/$(ENABLER) rootfs
	@ echo "Copy $(ENABLER)"
	@ cp $(BUILD_FOLDER)/$(ENABLER) $(ROOTFS_FOLDER)/usr/bin
	@ $(CROSS_COMPILE)strip $(ROOTFS_FOLDER)/usr/bin/$(ENABLER)
	# RAZA TODO - copy shared library libs and binaries
	@ mkdir -p $(ROOTFS_FOLDER)/lib/modules
	@ cp $(MODULES_FOLDER)/*.ko $(ROOTFS_FOLDER)/lib/modules

image: rootfs kernel
	@ mkdir -p $(ROOTFS_FOLDER)/boot
	@ cp $(KERNEL_BOOT_PATH)/$(KERNEL_IMAGE_NAME) $(ROOTFS_FOLDER)/boot
	@ cp $(KERNEL_DTB_FILE_PATH) $(ROOTFS_FOLDER)/boot
	# RAZA TODO - rename the image with configuration and version (use version_info?)
	 mkfs.ubifs -m 4KiB -e 248KiB -c 4000 -r $(ROOTFS_FOLDER) -x none -vv $(BUILD_FOLDER)/ubifs_256_.img
	@ mkfs.ubifs -m 4KiB -e 504KiB -c 2000 -r $(ROOTFS_FOLDER) -x none -vv $(BUILD_FOLDER)/ubifs_512.img
	@ echo "images ready at $(BUILD_FOLDER)"

