export ARCH ?= mips
WIDTH ?= 64
#DEFCONFIG ?= mvebu_lsp_defconfig
DEFCONFIG ?= cavium_octeon_defconfig
ADDITIONAL_DEFCONFIG := \
	$(KERNEL_FOLDER)/scripts/config --file $(KERNEL_FOLDER)/.config --disable CONFIG_DEBUG_INFO \
	 --disable CONFIG_MV_INCLUDE_SFLASH_MTD \
	 --disable CONFIG_MV_INCLUDE_PRESTERA_PCI \
	 --enable CONFIG_TUN

ifeq ("$(CROSS_COMPILE)", "")
  export CROSS_COMPILE := /swtools/devtools/gnueabi/mips/usr/bin/mips64-linux-gnuabi64-
  TARGET_SPECIFIC_CFLAGS := --sysroot=/swtools/devtools/gnueabi/mips
  $(info CROSS_COMPILE not specified, defaulting to $(CROSS_COMPILE))
endif
ifneq ("$(ARCH)", "mips")
  $(error ARCH must be armv8 for this target)
endif
ifneq ("$(WIDTH)", "64")
  $(error WIDTH must be 64 for this target)
endif
SHARED_LIB_LD_ARCH := MIPS64 #TODO - this is temporary for compatibility with build_cpss.sh, remove when not needed
OS_RUN ?= linux

# flags used by kernel modules makefile - need to change according to kernel version
export CPU_FAMILY := CPU_MIPS
export LSP_KERNEL_TYPE := Image



