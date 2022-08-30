ifeq ($(SIM),)
  CROSS_COMPILE ?= /swtools/devtools/gnueabi/arm_le/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu-
endif

CC = $(CROSS_COMPILE)gcc
APP_NAME := efuse

CFLAGS += -Wall -g -ansi -lm
LDFLAGS = -lpthread -lrt
CPSS_ROOT = $(realpath ../..)

ifneq ($(DEBUG),)
  CFLAGS += -DDEBUG
endif

ifneq ($(SIM),)
	CFLAGS += -DSIM
endif

ifneq ($(LINUX),)
	CFLAGS += -DLINUX
endif

SRCS := main.c fuselist.c

OBJS = $(notdir $(SRCS:.c=.o))

print-%: ; @echo $*=$($*)

all: $(APP_NAME)
	@ echo Done

%.o : %.c fuselist.h
	@echo CC $@
	$(CC) $(CFLAGS) -c $< -o $@

$(APP_NAME): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(APP_NAME)

clean:
	rm -f $(OBJS) $(APP_NAME)

print-%: ; @echo $*=$($*)
