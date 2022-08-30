export ARCH ?= x86_64
WIDTH ?= 64
CPU ?= i386
ASIC_SIMULATION := y
TARGET_SPECIFIC_CFLAGS := -DWIN32 -DIMAGE_HELP_SHELL -DAPPLICATION_SYMBOL_TABLE -DIMAGE_HELP_SHELL
#used to suppress warnings.
WARN_FLAGS := -Wno-error=format -Wno-error=unknown-pragmas -Wno-error=int-to-pointer-cast \
                -Wno-error=format-extra-args -Wno-error=pointer-to-int-cast -Wno-error=type-limits \
                -Wno-error=implicit-function-declaration -Wno-error=unused-but-set-variable -Wno-error=pointer-sign \
                -Wno-error=unused-variable -Wno-error=main
TARGET_SPECIFIC_CFLAGS += $(WARN_FLAGS)
OS_RUN := win32
TOOL_TYPE := gcc

ifneq ("$(ARCH)", "x86_64")
  $(error ARCH must be x86 for this target)
endif
ifneq ("$(WIDTH)", "64")
  $(error WIDTH must be 32 for this target)
endif
ifneq ("$(CPU)", "i386")
  $(error CPU must be i386 for this target)
endif
