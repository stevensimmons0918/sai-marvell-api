MODULES := 

FILES := \
 iDbgPdlPhy.c

CWD := $(dir $(lastword $(MAKEFILE_LIST)))
IDBG_PDL_SRC += $(patsubst %,$(CWD)%,$(FILES))
include $(patsubst %,$(CWD)%/$(MAKEFILE),$(MODULES))

