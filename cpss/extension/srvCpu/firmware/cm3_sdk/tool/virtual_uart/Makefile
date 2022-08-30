
# requirements:
# CROSS_COMPILE
# SHM_UART_BASE - offset in CM3 SRAM for virtual uart buffers
# SHM_UART_SIZE - size of a single virtual uart buffer. App uses 2 buffers (Rx,Tx) + a word for heartBeat

# CROSS_COMPILE := /swtools/devtools/gnueabi/arm_le/arm-none-linux-gnueabi-versions/aarch64v8-marvell-linux-gnu-5.2.1_x86_64_20151110/bin/aarch64-marvell-linux-gnu-
# CROSS_COMPILE := /swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi-

####

# Example How to Compile this Tool for Falcon (the vuart will give us CLI to one of the ravens):
# define MAKE alias:
# alias MAKE64='make ARCH=arm64 CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/arm-none-linux-gnueabi-versions/aarch64v8-marvell-linux-gnu-5.2.1_x86_64_20151110/bin/aarch64-marvell-linux-gnu- LOCALVERSION='
# Call to MAKE command:
# MAKE64 -s SHM_UART_BASE=0x1f000 SHM_UART_SIZE=256 WATCHDOG=y clean all
# WATCHDOG - should be the same flag also in SDK to support watchdog mechanism. Default is no
####

ifndef CROSS_COMPILE
  $(error missing param CROSS_COMPILE)
endif

CC = $(CROSS_COMPILE)gcc
APP_NAME := vuart


CFLAGS += -Wall -g -DUNIX -DLINUX -lpthread -DBUF_SIZE=$(BUF_SIZE)
LDFLAGS = -lpthread -lrt
CPSS_ROOT = $(realpath ../../..)
INCLUDE_PATH = $(CPSS_ROOT)/cpssEnabler/mainOs/h $(CPSS_ROOT)/common/h $(CPSS_ROOT)/common/h/cpss/extServices/os

CFLAGS += $(addprefix -I, $(INCLUDE_PATH))

ifneq (,$(PP))
  CFLAGS += -D$(PP)
endif

ifneq ($(MAKECMDGOALS), clean)
  ifdef SHM_UART_BASE
    CFLAGS += -DSHM_UART_BASE=$(SHM_UART_BASE)
  else
    $(error missing param SHM_UART_BASE)
  endif
  ifdef SHM_UART_SIZE
    CFLAGS += -DSHM_UART_SIZE=$(SHM_UART_SIZE)
  else
    $(error missing param SHM_UART_SIZE)
  endif
endif

ifeq ($(WATCHDOG),y)
  CFLAGS += -DWATCHDOG
endif

# define the C source files
SRCS := main.c terminal.c mvShmUart.c

OBJS = $(notdir $(SRCS:.c=.o))

print-%: ; @echo $*=$($*)

all: $(APP_NAME)
	@ echo Done

%.o : %.c
	@echo CC $@
	$(CC) $(CFLAGS) -c $< -o $@

$(APP_NAME): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(APP_NAME)

clean:
	rm -f $(OBJS) $(APP_NAME)

print-%: ; @echo $*=$($*)

