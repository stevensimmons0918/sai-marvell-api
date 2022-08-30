ARCH ?= arm
WIDTH ?= 32
CPU ?= arm
CPU_BOARD ?= arm
ASIC_SIMULATION := y
LINUX_SIM := 1
OS_RUN ?= linux
ifeq ("$(CROSS_COMPILE)", "")
  CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
  $(info CROSS_COMPILE not specified, defaulting to $(CROSS_COMPILE))
endif
ifneq ("$(ARCH)", "arm")
  $(error ARCH must be arm for this target)
endif
ifneq ("$(WIDTH)", "32")
  $(error WIDTH must be 32 for this target)
endif
