#
# pre-requisite to running this makefile-
# Build CAP LIB using the following options - CPSS_APP_LIB UTF_NO CPSS_LOG_NO
#

#sampleMain by default tests sim64 - please change target to armv8 test hw - the new make env. doesnt involve exporting simulation or other flags.
TARGET := sim64

BRANCH := /$(shell git rev-parse --abbrev-ref HEAD)
FAMILY := DX

ifeq (sim64, $(TARGET))
    BUILD_LINUX_SIMULATION = TRUE
else
    BUILD_LINUX_SIMULATION = FALSE
endif

ifeq (TRUE, $(BUILD_LINUX_SIMULATION))
CC = gcc
MACHINE = -m64
LDFLAGS = -L/usr/lib64 $(MACHINE) -lpthread -ldl -rdynamic -lrt -lm
LINKER_PASS_START =
LINKER_PASS_END =
else
#Aldrin2 32 bit
CC = /swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi-gcc
LDFLAGS = -L/swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/lib -lpthread -ldl -rdynamic -lrt -lm

#Aldrin 3.10 Kernel
CC = /swtools/devtools/gnueabi/arm_le/arm-none-linux-gnueabi-versions/armv7-marvell-linux-gnueabi-softfp_i686/bin/arm-marvell-linux-gnueabi-gcc
LDFLAGS = -L/swtools/devtools/gnueabi/arm_le/arm-none-linux-gnueabi-versions/armv7-marvell-linux-gnueabi-softfp_i686/lib -lpthread -ldl -rdynamic -lrt -lm

#Aldrin2 64bit
CC = /swtools/devtools/gnueabi/arm_le/aarch64v8-marvell-linux-gnu/bin/aarch64-marvell-linux-gnu-gcc
LDFLAGS = -L/swtools/devtools/gnueabi/arm_le/aarch64v8-marvell-linux-gnu/lib -lpthread -ldl -rdynamic -lrt -lm

LINKER_PASS_START = -Xlinker --whole-archive
LINKER_PASS_END = -Xlinker --no-whole-archive
endif

ENABLE_DEBUG ?= TRUE
#Note simulation is supported only on i386 -m32 platform

MACRO = -D CHX_FAMILY

ifeq ("$(LUA)","Y")
MACRO += -D LUA_CLI_SAMPLE
endif

CPSS_ROOT_PATH = ../../../..

BUILD_CPSS:= $(shell cd $(CPSS_ROOT_PATH); ./build_cpss_old.sh DX_ALL AARCH64v8 NOKERNEL CPSS_APP_LIB UTF_NO CPSS_LOG_NO)

ifndef COMPILATION_ROOT
COMPILATION_ROOT=$(CPSS_ROOT_PATH)/compilation_root$(BRANCH)/$(TARGET)_$(FAMILY)
endif

SUFFIX =
ifeq (TRUE, $(BUILD_LINUX_SIMULATION))
	MACRO += -DLINUX_SIMULATION
        MACRO += -DASIC_SIMULATION
        MACRO += -DLINUX_SIM
	SRCS = simulationInit.c
	SUFFIX = Sim
endif

#dont do for Simulation below one
ifeq (FALSE, $(BUILD_LINUX_SIMULATION))
MACRO += -DSHARED_MEMORY
MACRO += -DPOSIX_SEM
endif
MKDIR_COMMAND = mkdir -p

#include path needed for compilation
INCLUDE_PATH = -I $(CPSS_ROOT_PATH)/cpssEnabler/mainOs/h
INCLUDE_PATH += -I $(CPSS_ROOT_PATH)/cpssEnabler/mainCmd/h
INCLUDE_PATH += -I $(CPSS_ROOT_PATH)/common/h
INCLUDE_PATH += -I $(CPSS_ROOT_PATH)/common/h/cpssAppPlatform
INCLUDE_PATH += -I $(CPSS_ROOT_PATH)/cpssEnabler/cpssAppPlatform/h
INCLUDE_PATH += -I $(CPSS_ROOT_PATH)/mainPpDrv/h
INCLUDE_PATH += -I $(CPSS_ROOT_PATH)/mainExtUtils/h
INCLUDE_PATH += -I $(CPSS_ROOT_PATH)/common/h/cpss/extServices/os
ifeq (TRUE, $(BUILD_LINUX_SIMULATION))
INCLUDE_PATH += -I $(CPSS_ROOT_PATH)/simulation/simGeneral/h
INCLUDE_PATH += -I $(CPSS_ROOT_PATH)/simulation/simOs/h
else
INCLUDE_PATH += -I $(CPSS_ROOT_PATH)/cpssEnabler/mainExtDrv/h
INCLUDE_PATH += -I $(CPSS_ROOT_PATH)/cpssEnabler/mainExtUtils/h/extUtils/common
INCLUDE_PATH += -I $(CPSS_ROOT_PATH)/cpssEnabler/cpssAppPlatform/h/appReference
INCLUDE_PATH += -I $(CPSS_ROOT_PATH)/cpssEnabler/cpssAppPlatform/h/ezbringup
endif

CPSS_APP_PLAT_LIB_NAME = cpssAppPlatLib.a
CPSS_OS_CALLS_LIB_NAME = cpssAppPlatOsLib.a
CPSS_EXTDRV_CALLS_LIB_NAME = cpssAppPlatExtDrvLib.a

#Below .c file has  main() function definition
SRCS += sampleMain.c

#Event handling from AppRef
#REF_FILE_PATH = $(CPSS_ROOT_PATH)/cpssEnabler/cpssAppPlatform/src/appReference
#SRCS += $(REF_FILE_PATH)/cpssAppRefAuEventHandle.c
#SRCS += $(REF_FILE_PATH)/cpssAppRefGenEventHandle.c

#Profiles
#PROFILE_FILE_PATH =  $(CPSS_ROOT_PATH)/cpssEnabler/cpssAppPlatform/src/profiles
#SRCS += $(PROFILE_FILE_PATH)/boardProfiles/Board-DB-98EX5520-6ZQSFP-48SFP.c
#SRCS += $(PROFILE_FILE_PATH)/mainProfiles/DB-98EX5520-6ZQSFP-48SFP-sample.c
#SRCS += $(PROFILE_FILE_PATH)/ppProfiles/PP-98EX5520.c
#SRCS += $(PROFILE_FILE_PATH)/runTimeProfiles/runTimeProfile_aldrin.c

CFLAGS = $(MACHINE) $(INCLUDE_PATH) -Wall  $(MACRO)
ifeq (TRUE, $(ENABLE_DEBUG))
CFLAGS += -g
endif

CFLAGS += -DLINUX
ifeq (TRUE, $(BUILD_LINUX_SIMULATION))
CFLAGS += -DASIC_SIMULATION -DLINUX_SIMULATION
endif

CPSS_APP_PLAT_LIB = $(COMPILATION_ROOT)/$(CPSS_APP_PLAT_LIB_NAME)

CPSS_OS_CALLS_LIB = $(COMPILATION_ROOT)/$(CPSS_OS_CALLS_LIB_NAME)
CPSS_EXTDRV_CALLS_LIB = $(COMPILATION_ROOT)/$(CPSS_EXTDRV_CALLS_LIB_NAME)

USER_APPLICATION_NAME = cpssAppPlatMain$(SUFFIX)

all:
	$(CC) $(CFLAGS) $(SRCS) $(LINKER_PASS_START) $(COMPILATION_ROOT)/cpssEnabler/libs/CPSSAPPREF.a $(COMPILATION_ROOT)/cpssEnabler/libs/DEMO.a $(COMPILATION_ROOT)/cpssEnabler/libs/EXTPHYM.a $(COMPILATION_ROOT)/cpssEnabler/libs/UTFWRAPPERS.a $(COMPILATION_ROOT)/mainPpDrv/libs/MAINPPDRV.a $(COMPILATION_ROOT)/common/libs/COMMON.a $(COMPILATION_ROOT)/mainPpDrv/libs/MAINPPDRV.a $(COMPILATION_ROOT)/mainTmDrv/libs/MAINTMDRV.a $(COMPILATION_ROOT)/mainExtUtils/libs/MAINEXTUTILS.a $(COMPILATION_ROOT)/mainLuaWrapper/libs/MAINLUAWRAPPER.a $(COMPILATION_ROOT)/simulation/libs/SIMULATION.a $(COMPILATION_ROOT)/cpssEnabler/libs/MAINCMD.a $(COMPILATION_ROOT)/cpssEnabler/libs/GTOS.a $(COMPILATION_ROOT)/cpssEnabler/libs/GTUTIL.a $(COMPILATION_ROOT)/cpssEnabler/libs/GTSTACK.a $(COMPILATION_ROOT)/cpssEnabler/libs/MAINEXTDRV.a $(LINKER_PASS_END) -o $(USER_APPLICATION_NAME) $(LDFLAGS)

clean:
	rm -f *.o $(USER_APPLICATION_NAME)
