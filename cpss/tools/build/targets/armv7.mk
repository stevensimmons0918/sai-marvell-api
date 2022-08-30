export ARCH ?= arm
WIDTH ?= 32
ifeq ("$(CROSS_COMPILE)", "")
  export CROSS_COMPILE := /swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi-
  $(info CROSS_COMPILE not specified, defaulting to $(CROSS_COMPILE))
endif
ifneq ("$(ARCH)", "arm")
  $(error ARCH must be arm for this target)
endif
ifneq ("$(WIDTH)", "32")
  $(error WIDTH must be 32 for this target)
endif
SHARED_LIB_LD_ARCH := ARMARCH7#TODO - this is temporary for compatibility with build_cpss.sh, remove when not needed
OS_RUN ?= linux

# flags used by kernel modules makefile - need to change according to kernel version
export CPU_FAMILY := CPU_ARM
export LSP_KERNEL_TYPE := zImage# for backward compatibility
export KERNEL_IMAGE_NAME := zImage

DEFCONFIG ?= mvebu_v7_lsp_defconfig
ADDITIONAL_DEFCONFIG = \
	$(KERNEL_FOLDER)/scripts/config --file $(KERNEL_FOLDER)/.config --disable CONFIG_DEBUG_INFO \
	 --disable CONFIG_MV_INCLUDE_SFLASH_MTD \
	 --disable CONFIG_MV_INCLUDE_PRESTERA_PCI \
	 --enable CONFIG_TUN

DEVICE_TREE_FILE ?= armada-38x-interposer.dts armada-390-db.dts armada-385-amc.dts msys-ac3-db.dts msys-ac3-rd.dts
#armada-385-customer3.dts - for kernel 3.10

LINUX_KERNEL_REPO := vgitil04.il.marvell.com:29418/syssw/kernel
LINUX_KERNEL_COMMIT := 523e73bf1c4eab07e460efe9da6dbb192c33f998  #linux-4.4.52-devel, devel-18.02.3

