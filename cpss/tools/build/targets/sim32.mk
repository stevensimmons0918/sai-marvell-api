export ARCH ?= x86
WIDTH ?= 32
CPU ?= i386
CONFIG_ASIC_SIMULATION := y
LINUX_SIM := 1
OS_RUN ?= linux
TARGET_SPECIFIC_CFLAGS := -m$(WIDTH)

ifneq ("$(ARCH)", "x86")
  $(error ARCH must be x86 for this target)
endif
ifneq ("$(WIDTH)", "32")
  $(error WIDTH must be 32 for this target)
endif
ifneq ("$(CPU)", "i386")
  $(error CPU must be i386 for this target)
endif
SHARED_LIB_LD_ARCH := sim32 #TODO - this is temporary for compatibility with build_cpss.sh, remove when not needed



