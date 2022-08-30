MODULES :=

FILES := \
 mydAPI.c \
 mydAPIInternal.c \
 mydDiagnostics.c \
 mydFEC.c \
 mydFwDownload.c \
 mydHwCntl.c \
 mydHwSerdesCntl.c \
 mydInitialization.c \
 mydUtils.c \
 mydIntrIOConfig.c

ifneq (EXISTS, $(PHY_SIMULATION)) 
 FILES += \
  serdes/avago/aapl/aacs.c \
  serdes/avago/aapl/aacs_server.c \
  serdes/avago/aapl/aapl_core.c \
  serdes/avago/aapl/avsp.c \
  serdes/avago/aapl/diag_core.c \
  serdes/avago/aapl/dfe.c \
  serdes/avago/aapl/eye.c \
  serdes/avago/aapl/eye_math.c \
  serdes/avago/aapl/gpio_mdio.c \
  serdes/avago/aapl/i2c.c \
  serdes/avago/aapl/library.c \
  serdes/avago/aapl/logging.c \
  serdes/avago/aapl/mdio.c \
  serdes/avago/aapl/meas.c \
  serdes/avago/aapl/pmd.c \
  serdes/avago/aapl/pmro.c \
  serdes/avago/aapl/sbm.c \
  serdes/avago/aapl/sbus.c \
  serdes/avago/aapl/sensor.c \
  serdes/avago/aapl/serdes_core.c \
  serdes/avago/aapl/spico.c \
  serdes/avago/aapl/spico_diag.c
endif

CWD := $(dir $(lastword $(MAKEFILE_LIST)))
CFLAGS := $(CFLAGS) -I$(CWD)
include $(patsubst %,$(CWD)%/$(MAKEFILE),$(MODULES))
PHY_SRC += $(patsubst %,$(CWD)%,$(FILES))
