export ARCH ?= x86
WIDTH ?= 32
CPU ?= i386
ASIC_SIMULATION := y
TARGET_SPECIFIC_CFLAGS := -DWIN32 -DIMAGE_HELP_SHELL -DAPPLICATION_SYMBOL_TABLE -DIMAGE_HELP_SHELL
OS_RUN := win32

ifneq ("$(ARCH)", "x86")
  $(error ARCH must be x86 for this target)
endif
ifneq ("$(WIDTH)", "32")
  $(error WIDTH must be 32 for this target)
endif
ifneq ("$(CPU)", "i386")
  $(error CPU must be i386 for this target)
endif
#ifneq ("$(CPU_BOARD)", "i386")
#  $(error CPU_BOARD must be i386 for this target)
#endif
