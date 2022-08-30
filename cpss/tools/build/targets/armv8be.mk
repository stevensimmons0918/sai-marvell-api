export ARCH ?= arm64
WIDTH ?= 64
TARGET_SPECIFIC_CFLAGS := -mbig-endian -DCPU_BE

ifeq ("$(CROSS_COMPILE)", "")
  export CROSS_COMPILE := /swtools/devtools/gnueabi/arm_be/aarch64ebv8-marvell-linux-gnu/bin/aarch64_be-linux-gnu-
  $(info CROSS_COMPILE not specified, defaulting to $(CROSS_COMPILE))
endif
ifneq ("$(ARCH)", "arm64")
  $(error ARCH must be arm for this target)
endif
ifneq ("$(WIDTH)", "64")
  $(error WIDTH must be 32 for this target)
endif
SHARED_LIB_LD_ARCH := AARCH64v8 #TODO - this is temporary for compatibility with build_cpss.sh, remove when not needed
OS_RUN ?= linux

# flags used by kernel modules makefile - need to change according to kernel version
export CPU_FAMILY := CPU_ARM
export LSP_KERNEL_TYPE := Image# for backward compatibility
export KERNEL_IMAGE_NAME := Image

DEFCONFIG ?= mvebu_v8_lsp_defconfig
ADDITIONAL_DEFCONFIG := \
	$(KERNEL_FOLDER)/scripts/config --file $(KERNEL_FOLDER)/.config --disable CONFIG_DEBUG_INFO \
	 --disable CONFIG_MV_INCLUDE_SFLASH_MTD \
	 --disable CONFIG_MV_INCLUDE_PRESTERA_PCI \
	 --enable CONFIG_CPU_BIG_ENDIAN \
	 --enable CONFIG_CPU_ENDIAN_BE64 \
	 --enable CONFIG_TUN

DEVICE_TREE_FILE ?= armada-7020-amc.dts armada-7020-comexpress.dts

LINUX_KERNEL_REPO := vgitil04.il.marvell.com:29418/syssw/kernel
LINUX_KERNEL_COMMIT := 523e73bf1c4eab07e460efe9da6dbb192c33f998  #linux-4.4.52-devel, devel-18.02.3

