ARCH ?= x86
WIDTH ?= 32
CPU ?= i386
CPU_BOARD ?= i386_64
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
ifneq ("$(CPU_BOARD)", "i386_64")
  $(error CPU_BOARD must be i386 for this target)
endif
SHARED_LIB_LD_ARCH := x86
OS_RUN ?= linux

# DEFCONFIG ?= mvebu_lsp_defconfig
# ADDITIONAL_DEFCONFIG :=
# DEVICE_TREE_FILE ?= armada-385-customer3.dts


