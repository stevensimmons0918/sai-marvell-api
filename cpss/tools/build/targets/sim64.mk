ARCH ?= x86
WIDTH ?= 64
CPU ?= i386
CPU_BOARD ?= i386_64
CONFIG_ASIC_SIMULATION := y
LINUX_SIM := 1
OS_RUN ?= linux

ifneq ("$(ARCH)", "x86")
  $(error ARCH must be x86 for this target)
endif
ifneq ("$(WIDTH)", "64")
  $(error WIDTH must be 32 for this target)
endif
ifneq ("$(CPU)", "i386")
  $(error CPU must be i386 for this target)
endif
ifneq ("$(CPU_BOARD)", "i386_64")
  $(error CPU_BOARD must be i386 for this target)
endif
SHARED_LIB_LD_ARCH := sim64 #TODO - this is temporary for compatibility with build_cpss.sh, remove when not needed

# DEFCONFIG ?= mvebu_lsp_defconfig
# ADDITIONAL_DEFCONFIG := 
# DEVICE_TREE_FILE ?= armada-385-customer3.dts


