#
## marvell GPIO Driver AC3x
#

#TARGETS = mvGpioDrv

#ccflags-y := -I.

#obj-m += $(addsuffix .o,$(TARGETS))

#mvGpioDrv-y := mvGpioDrv.o

LINUX ?= 5.10
LINUX_PATH ?= linux-5.10.46

ifeq ($(LINUX),4.9)
LINUX_PATH = linux-4.19.152
endif

obj-m += mvGpioDrv.o

all:
	make -C /sonic/src/sonic-linux-kernel/$(LINUX_PATH)/debian/build/build_armhf_none_armmp/ M=$(PWD) modules

clean:
	make -C /sonic/src/sonic-linux-kernel/$(LINUX_PATH)/debian/build/build_armhf_none_armmp/ M=$(PWD) clean
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions modules.order Module.symvers

