MODULES := \
 init \
 logger \
 button \
 utils \
 sensor \
 phy \
 serdes \
 packetProcessor \
 inband \
 fan \
 led \
 oob \
 cpu \
 interface \
 sfp \
 power \
 B2b
  
 
FILES := 

CWD := $(dir $(lastword $(MAKEFILE_LIST)))
IDBG_PDL_SRC += $(patsubst %,$(CWD)%,$(FILES))
include $(patsubst %,$(CWD)%/$(MAKEFILE),$(MODULES))

