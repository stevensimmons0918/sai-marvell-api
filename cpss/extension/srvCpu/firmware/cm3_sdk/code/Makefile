# requirements:
# CROSS_COMPILE
# SHM_UART - if y, shm-uart is enabled, default is no
# SHM_UART_BASE - base address in CM3 SRAM for shm-uart
# SHM_UART_SIZE - size of a single shm-uart ring (Rx/Tx). Both rings occupy x2
# QSPI - if y, QSPI is enabled, default is no
# SDK_DEBUG - if y, SDK_DEBUG options is enabled, default is no
# WATCHDOG - if y,  watchdog mechanism is enabled, default is no
# XOR_DMA - if y, XOR_DMA is enabled, default is no


CROSS_COMPILE?=/swtools/devtools/gnueabi/arm_le/arm-none-linux-gnueabi-versions/gcc-arm-none-eabi-4_9-2015q1/bin/arm-none-eabi-
APP_NAME := cm3_sdk

#
# Host configuration
#
CPU = cortex-m3

DEP :=
CPSS_DIR := $(realpath $(shell pwd)/../../../../..)
SRV_CPU_DIR = $(CPSS_DIR)/extension/srvCpu
COMMON_H_DIR = $(CPSS_DIR)/common/h
COMMON_DRIVERS_DIR =  $(SRV_CPU_DIR)/common/drivers
RTOS_DIR =  $(SRV_CPU_DIR)/common/FreeRTOS/FreeRTOS
RTOS_SOURCE_DIR = $(RTOS_DIR)/Source
PORT_DIR = $(RTOS_SOURCE_DIR)/portable/GCC
PORT_CM3_DIR = $(PORT_DIR)/ARM_CM3
TWSI_DIR = $(SRV_CPU_DIR)/common/drivers/peripherals/i2c
RTOS_PLUS_DIR = $(SRV_CPU_DIR)/common/FreeRTOS/FreeRTOS-Plus
$(info SRV_CPU_DIR = $(SRV_CPU_DIR))
BUILD_DIR := objects

HEADER_BUILD = ../ARM_CM3_GCC/header_build

CFLAGS += -DCONFIG_BC3 -DCM3

# Commands for calling the compiler, archiver, linker...
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar
OBJCOPY = $(CROSS_COMPILE)objcopy
ELFSIZE = $(CROSS_COMPILE)size
MAKE = make

#
# CFLAGS common to both the THUMB and ARM mode builds
#
WARNINGS = -Wall -Wshadow -Wpointer-arith -Wbad-function-cast \
		 -Wcast-align -Wsign-compare -Waggregate-return -Wstrict-prototypes \
		 -Wmissing-prototypes -Wmissing-declarations -Wunused
       
# debug target. usage: make print-var will print value of variable "var"
print-%  : ; @echo $* = $($*)

CFLAGS += $(WARNINGS) -mcpu=$(CPU) -MD -D inline= \
		 -ffunction-sections -g -Os -s \
		 -I . -I $(PORT_CM3_DIR) -I $(PORT_CM3_DIR)/hw -I $(TWSI_DIR) \
		 -I $(RTOS_SOURCE_DIR)/include \
		 -I $(RTOS_PLUS_DIR)/ -I $(COMMON_H_DIR) \
		 -I $(CPSS_DIR)/mainPpDrv/h \
         -I $(COMMON_DRIVERS_DIR)/operationDrivers \
         -I $(SRV_CPU_DIR)/firmware/cm3_sdk/code/h \
         -I $(COMMON_DRIVERS_DIR)/peripherals/spi \
         -I $(COMMON_DRIVERS_DIR)/peripherals/timer \
         -I $(COMMON_DRIVERS_DIR)/infrastructure/doorbell \
         -I $(COMMON_DRIVERS_DIR)/peripherals/watchdog \
         -I $(COMMON_DRIVERS_DIR)/peripherals/xorDma \
         -I $(COMMON_DRIVERS_DIR)/peripherals/gpio

CFLAGS += -DTHUMB_INTERWORK
THUMB_FLAGS = -mthumb -Wa,-mthumb -mno-thumb-interwork -mfpu=vfp -msoft-float  -mfix-cortex-m3-ldrd
THUMB_LD_FLAGS = -mthumb

ifeq ($(DBG),)
  CFLAGS += -DFREE_RTOS_HWS_ENHANCED_PRINT_MODE
endif

ifeq ($(DISABLE_CLI),y)
  CFLAGS += -DDISABLE_CLI
endif

ENTRY = Reset
LINKER_FLAGS = -T RTOSDemo-cm3.ld --entry $(ENTRY) -Map RTOSDemo-cm3.map --gc-sections

#enforce C89-style variable declarations in gcc
CFLAGS += -Werror=declaration-after-statement
#forbids variable len arrays, C89-style, prevent compilation fail with VC10
CFLAGS += -Werror=vla

CFLAGS += -DMV_HWS_REDUCED_BUILD -DMV_HWS_REDUCED_BUILD_EXT_CM3

#
# Source files that can be built to THUMB mod.
#
SRC = \
  $(CURDIR)/src/main.c \
  $(COMMON_DRIVERS_DIR)/operationDrivers/mv_services.c \
  $(CURDIR)/src/cli.c \
  $(COMMON_DRIVERS_DIR)/infrastructure/ipc.c \
  $(COMMON_DRIVERS_DIR)/infrastructure/doorbell/doorbell.c \
  $(CURDIR)/src/printf.c \
  $(COMMON_DRIVERS_DIR)/peripherals/timer/mvOsTimer_cm3.c \
  $(COMMON_DRIVERS_DIR)/peripherals/gpio/mvGpio.c \
  $(COMMON_DRIVERS_DIR)/peripherals/spi/mvSpi.c \
  $(TWSI_DIR)/mvTwsi.c \
  $(CURDIR)/src/i2c_shell.c \
  $(CURDIR)/src/strtoul.c \
  $(PORT_CM3_DIR)/exceptions.c \
  $(PORT_CM3_DIR)/port.c \
  $(PORT_CM3_DIR)/heap.c \
  $(PORT_CM3_DIR)/hw/ic.c \
  $(RTOS_SOURCE_DIR)/list.c \
  $(RTOS_SOURCE_DIR)/queue.c \
  $(RTOS_SOURCE_DIR)/tasks.c \
  $(RTOS_SOURCE_DIR)/timers.c \
  $(RTOS_PLUS_DIR)/FreeRTOS_CLI.c \
  $(CPSS_DIR)/common/src/cpss/generic/ipc/mvShmIpc.c

ifeq ($(SHM_UART),y)
  SRC += $(PORT_CM3_DIR)/hw/mvShmUart.c
  CFLAGS += -DSHM_UART
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
else
  SRC += $(PORT_CM3_DIR)/hw/uart.c
endif

# Use DEBUG options
ifeq ($(SDK_DEBUG),y)
  CFLAGS += -DSDK_DEBUG
endif

ifeq ($(XOR_DMA),y)
	SRC += $(COMMON_DRIVERS_DIR)/peripherals/xorDma/xorDma.c
	CFLAGS += -DXOR_DMA
endif

ifeq ($(QSPI),y)
    SRC += $(CURDIR)/src/qspi_shell.c
    SRC += $(COMMON_DRIVERS_DIR)/peripherals/spi/mv_qspi.c
    CFLAGS += -DQSPI
endif

ifeq ($(WATCHDOG),y)
  SRC += $(COMMON_DRIVERS_DIR)/peripherals/watchdog/watchdog.c
  CFLAGS += -DWATCHDOG
endif

SRC := $(realpath $(SRC))

#
# Define all object files.
#
OBJ = $(patsubst $(realpath $(CPSS_DIR))/%.c,$(BUILD_DIR)/%.o,$(SRC))
DEP = $(OBJ:.o=.d)

#
# Get the location of libgcc.a and libc.a
#
LIBGCC=$(shell $(CC) $(THUMB_LD_FLAGS) -mcpu=$(CPU) -print-libgcc-file-name)
LIBC=$(shell $(CC) $(THUMB_LD_FLAGS) -mcpu=$(CPU) -print-file-name=libc.a)

HDR_IPC_FLAGS=-S 0x2f000 -s 0x1000

#-----------------------------------------------------------------
# Function compile(cflags, output_folder)
#
# Compile a .c file into .o file using the defined toolchain, including dependecy .d file.
# Object and dependency files will be located together in output_folder.
# dep_cflags     - CFLAGS to use for creating dependency file
# compile_cflags - CFLAGS to use for creating object file
# output_folder  - object file ouptut folder path prefix. Only files
#                  which should be created in this folder, will use
#                  this recipe.
#
# Example: $(Ocall compile $(DEP_CFLAGS),$(CFLAGS),/local/objects)
# Targets: /local/objects/foo/bar/baz.o
#          /local/objects/foo/bar/baz.d
# Source file: foo/bar/baz.c
#-----------------------------------------------------------------

define compile
@# create object folder
	@mkdir -p $(dir $@)
@# create dependency file
	$(CC) -M -c $1 $< -o$2/$*.d
@# Duplicate it
	cp $2/$*.d $2/$*.tmp
@# Create empty rule per file in dependency rule to avoid "No rule to make target ..." Errors
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/d' -e 's/$$/ :/' < $2/$*.d >> $2/$*.tmp
@# Change object filename to include full path
	sed -e 's/$(subst .,\.,$(notdir $@))/$(subst /,\/,$@)/'  < $2/$*.tmp > $2/$*.d
@# Remove temporary file
	rm $2/$*.tmp
@# print compilation msg
	@ echo "  CC $<"
@# compile
	$(CC) -c $1 $< -o$@
endef


#
# The default rule
#
all: $(APP_NAME).bin

$(APP_NAME).bin : $(APP_NAME).elf
	@$(OBJCOPY) -O binary $(APP_NAME).elf $@

$(APP_NAME).elf : $(OBJ)
	@echo "  LD    $@";
	@$(LD) $(LINKER_FLAGS) -o $@ $(OBJ) '$(LIBC)' '$(LIBGCC)'
	@SIZE=`$(ELFSIZE) $@ | tail -1|  cut -f 4`; \
	printf "    *** \033[34;1m.text+.data+.bss size:  0x%x \033[0m\n" $$SIZE

$(BUILD_DIR)/%.o: $(realpath $(CPSS_DIR))/%.c
	$(call compile,$(THUMB_FLAGS) $(CFLAGS),$(BUILD_DIR))

clean :
	@rm -rf ./$(BUILD_DIR) $(APP_NAME).elf $(APP_NAME).bin

# debug target. usage: make print-var will print value of variable "var"
print-%  : ; @echo $* = $($*)


-include $(DEP)



