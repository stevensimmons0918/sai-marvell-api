MODULES := 

FILES := \
 mtdDiagnosticsRegDumpData.c \
 mtdEEE.c \
 mtdHwSerdesCntl.c \
 TuPtp/mtdTuPtp.c \
 TuPtp/mtdTuPtpInternal.c \
 mtdHXunit.c \
 mtdHwCntl.c \
 mtdAPIInternal.c \
 mtdCunit.c \
 mtdUtils.c \
 mtdHunit.c \
 mtdXgFastRetrain.c \
 mtdInitialization.c \
 mtdIntr.c \
 mtdXunit.c \
 mtdFwDownload.c \
 fwimages/mtdFwDlSlave.c \
 fwimages/mtdFwImage.c \
 serdes/mcesd/mcesdInitialization.c \
 serdes/mcesd/mcesdUtils.c \
 serdes/mcesd/C28GP4X2/mcesdC28GP4X2_HwCntl.c \
 serdes/mcesd/C28GP4X2/mcesdC28GP4X2_API.c \
 serdes/mcesd/C28GP4X2/mcesdC28GP4X2_RegRW.c \
 serdes/mcesd/C28GP4X2/mcesdC28GP4X2_DeviceInit.c \
 serdes/mcesd/C28GP4X2/mcesdC28GP4X2_FwDownload.c \
 mtdDiagnostics.c \
 mtdAPI.c \
 LinkCryptPtp/mtdPtpInternal.c \
 LinkCryptPtp/mtdHwMsecPtpCntl.c \
 LinkCryptPtp/mtdPtpInit.c \
 LinkCryptPtp/aeskey.c \
 LinkCryptPtp/mtdPtpApi.c \
 LinkCryptPtp/mtdPtpConst.c \
 LinkCryptPtp/aescrypt.c \
 LinkCryptPtp/mtdMsecApi.c \
 LinkCryptPtp/mtdPtpUtils.c \
 LinkCryptPtp/mtdMsecInternal.c \
 LinkCryptPtp/aestab.c \
 LinkCryptPtp/mtdMsecEEE.c 


CWD := $(dir $(lastword $(MAKEFILE_LIST)))
CFLAGS := $(CFLAGS) -I$(CWD)
include $(patsubst %,$(CWD)%/$(MAKEFILE),$(MODULES))
PHY_SRC += $(patsubst %,$(CWD)%,$(FILES))




