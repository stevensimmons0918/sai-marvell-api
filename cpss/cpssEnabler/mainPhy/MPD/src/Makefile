MODULES := 

FILES := \
mpd.c \
mpdInit.c \
mpdUtils.c \
mpdmtd.c \
mpdmyd.c

ifeq ("$(DEBUG_FLAG)", "yes")
FILES += mpdDebug.c
else 
FILES += mpdDebugStub.c
endif 

CWD := $(dir $(lastword $(MAKEFILE_LIST)))
PHY_SRC += $(patsubst %,$(CWD)%,$(FILES))
include $(patsubst %,$(CWD)%/$(MAKEFILE),$(MODULES))

