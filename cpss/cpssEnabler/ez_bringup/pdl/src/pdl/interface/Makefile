MODULES := 

FILES := \
 pdlGpio.c \
 pdlI2c.c \
 pdlInterface.c \
 pdlSmiXsmi.c
 

CWD := $(dir $(lastword $(MAKEFILE_LIST)))
PDL_SRC += $(patsubst %,$(CWD)%,$(FILES))
include $(patsubst %,$(CWD)%/$(MAKEFILE),$(MODULES))

