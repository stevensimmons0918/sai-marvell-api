########################################
# this $(PRODUCT).mk file has stuff specific to $(PRODUCT)
########################################

PRODUCT := $(FILENAME_BASE)

$(PRODUCT): $(DIST)/xpsApp_py $(DIST)/_xpsApp.so $(DIST)/xpsApp.py

################################################################################
# we include dir.mk files in these subdirectories
# they recursively include dir.mk files below them
#
# if this product depends on another product(s) to build
# do not list their directories that are not otherwise needed for us 
#
# in <subdir>/dir.mk files are directory-specific variables and make targets, prepended with <subdir>/
# for example:
# in whitemodel/sde/mre 
# we have variables like	whitemodel/sde/mre/FOOBAR
# and targets like	whitemodel/sde/mre/foobar
#
# 
# for this Python interface, include all directories that build the product that you interface to
# 
# the Python-C++ bridge code is generated automatically so you don’t need to include any directory for that
################################################################################
SUBDIRS.$(PRODUCT) := \
	$(REL_TOP)/utils/xp \
	$(REL_TOP)/utils/xp/ipc \
	$(REL_TOP)/utils/xps \
	$(REL_TOP)/device/sdksrc/xp/fl \
	$(REL_TOP)/device/sdksrc/xp/dm/tableManager \
	$(REL_TOP)/device/sdksrc/xp/dm/devices/common/hwShadow \
	$(REL_TOP)/device/sdksrc/xp/dm
ifeq ($(findstring Y, $(XP70A0) $(XP70A1) $(XP70A2)),Y)
SUBDIRS.$(PRODUCT) += \
	$(REL_TOP)/device/sdksrc/xp/dm/tableManager/devices/xp70 \
	$(REL_TOP)/device/sdksrc/xp/dm/devices/xp70 \
	$(REL_TOP)/device/sdksrc/xp/dm/devices/xp70/A0 \
	$(REL_TOP)/device/sdksrc/xp/dm/devices/xp70/A0/hwShadow
endif
ifdef XP100
SUBDIRS.$(PRODUCT) += \
	$(REL_TOP)/device/sdksrc/xp/dm/tableManager/devices/xp100 \
	$(REL_TOP)/device/sdksrc/xp/dm/devices/xp100 \
	$(REL_TOP)/device/sdksrc/xp/dm/devices/xp100/A0 \
	$(REL_TOP)/device/sdksrc/xp/dm/devices/xp100/A0/hwShadow
endif
ifeq ($(findstring Y, $(XP80A0) $(XP80B0)),Y)
SUBDIRS.$(PRODUCT) += \
	$(REL_TOP)/device/sdksrc/xp/dm/tableManager/devices/xp80 \
	$(REL_TOP)/device/sdksrc/xp/dm/devices/xp80
endif
ifdef XP80A0
SUBDIRS.$(PRODUCT) += \
	$(REL_TOP)/device/sdksrc/xp/dm/devices/xp80/A0 \
	$(REL_TOP)/device/sdksrc/xp/dm/devices/xp80/A0/hwShadow
endif
ifdef XP80B0
SUBDIRS.$(PRODUCT) += \
	$(REL_TOP)/device/sdksrc/xp/dm/devices/xp80/B0 \
	$(REL_TOP)/device/sdksrc/xp/dm/devices/xp80/B0/hwShadow
endif
SUBDIRS.$(PRODUCT) += \
	$(REL_TOP)/utils \
	$(REL_TOP)/device/sdksrc/xp/entryFormats \
	$(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/entryFormats \
	$(REL_TOP)/pipeline-profiles/xpDefault \
	$(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xps \
        $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/cInterface/fl/copp/include \
	$(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/cInterface/fl \
	$(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/fl \
	$(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/pl \
	$(REL_TOP)/device/sdksrc/xp/pl \
	$(REL_TOP)/device/sdksrc/xp/dm/devices \
	$(REL_TOP)/utils/xp/include \
	$(REL_TOP)/utils/xp/ds/include \
	$(REL_TOP)/utils/xp/ipc/include \
	$(REL_TOP)/device/sdksrc/xp/system/init \
	$(REL_TOP)/device/sdksrc/xp/cInterface \
	$(REL_TOP)/device/sdksrc/xp/xpDiag \


SUBDIRS.$(PRODUCT) += \
	$(REL_TOP)/build/SWIG \
	$(REL_TOP)/pipeline-profiles/xpDefault/demo/common \
	$(REL_TOP)/pipeline-profiles/xpDefault/demo/common/py \
	$(REL_TOP)/pipeline-profiles/xpDefault/demo/config \
	$(REL_TOP)/pipeline-profiles/xpDefault/demo/xpsApp \
	$(REL_TOP)/pipeline-profiles/xpDefault/demo/xpsApp/l2 \
	$(REL_TOP)/pipeline-profiles/xpDefault/demo/xpsApp/l3 \
	$(REL_TOP)/device/sdksrc/xp/plCInterface \
	$(REL_TOP)/device/sdksrc/xp/cInterface/pl \

#ifndef MAC
	SUBDIRS.$(PRODUCT) += $(REL_TOP)/device/sdksrc/xp/system/mgmt \
	$(REL_TOP)/device/sdksrc/xp/system/hal/driverWrapper \
	$(REL_TOP)/device/sdksrc/xp/system/hal/interruptManager \
	$(REL_TOP)/device/sdksrc/xp/system/sal/ \
	$(REL_TOP)/device/sdksrc/xp/system/sal/bufferMgr \
	$(REL_TOP)/device/sdksrc/xp/system/sal/dmaAllocator \
	$(REL_TOP)/device/sdksrc/xp/system/sal/driverWrapper \

#endif





SUBDIRS.$(PRODUCT) += \
        $(REL_TOP)/pipeline-profiles/pure-openflow

SUBDIRS.$(PRODUCT) += \
        $(REL_TOP)/pipeline-profiles/segmentRoute

ifdef TEST_WITH_HW
SUBDIRS.$(PRODUCT) += $(REL_TOP)/device/sdksrc/xp/system/mgmt/os/linux/src
else
SUBDIRS.$(PRODUCT) += $(REL_TOP)/device/sdksrc/xp/system/mgmt/os/wmTest/src
endif

SINGLE_SRCS.$(PRODUCT) := \
	$(REL_TOP)/build/SWIG/prod_ifs/sdk.i

################################################################################
# compiler options to preprocess all front-end files
################################################################################
INCLUDES.$(PRODUCT) := \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/pl/include 				\
	-iquote $(REL_TOP)/device/sdksrc/xp/pl/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/pl/managers/include \
	-iquote $(REL_TOP)/utils/xp/include 			\
	-iquote $(REL_TOP)/utils/xp/ds/include 			\
	-iquote $(REL_TOP)/utils/xp/ipc/include 			\
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/include 			\
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/fl/copp/include \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/fl/int/include \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/acl/include       \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/include       \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/fl/of/include               \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/openflow/include \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include 			\
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/pl/packers/devices/common/include 			\
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/pl/packers/devices/xp80/include 			\
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/pl/packers/devices/xp70/include 			\
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/pl/packers/devices/xp100/include 			\
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/tableManager/include 			\
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/tableManager/devices/xp80/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/tableManager/devices/xp70/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/devices/xp70/A0/hwShadow/include		\
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/devices/xp70/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/tableManager/devices/xp100/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/devices/xp100/hwShadow/include		\
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/devices/xp100/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/devices/xp80/hwShadow/include		\
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/devices/xp80/A0/hwShadow/include		\
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/devices/xp80/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/devices/xp80/B0/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/devices/xp80/B0/hwShadow/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/devices/xp80/A0/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/config/range/include				\
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/profiles/devices/xp80/range/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/devices/common/hwShadow/include		\
	-iquote $(REL_TOP)/device/sdksrc/xp/system/init/profiles/include \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/fl/l2/include \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/fl/l3/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/fl/init/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/fl/profileMgr/include \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/fl/tunnel/include \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/fl/acl/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/fl/acm/include \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/fl/multicast/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/fl/age/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/fl/allocator/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/fl/mirror/include \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/fl/nat/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/fl/qos/include \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/fl/debug/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/fl/ptp/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/fl/interface/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/include \
	-iquote $(REL_TOP)/device/sdksrc/xps/include \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xps/include/ \
	-iquote $(REL_TOP)/utils/xps/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/devices/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/devices/xp80/A0/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/devices/xp80/A0/profiles/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/devices/xp80/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/devices/xp80/profiles/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/devices/xp80/profiles/urw/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/devices/common/config/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/devices/common/include \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/profiles/devices/xp80/urw/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/devices/xp80/profiles/urw/include/ \
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/devices/xp80/config/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/fl/linkManager/aapl/aapl/src/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/fl/linkManager/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/system/mgmt/packetDriver/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/system/mgmt/messageDriver/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/system/mgmt/ipc/include  \
	-iquote $(REL_TOP)/device/sdksrc/xp/system/mgmt/include  \
	-iquote $(REL_TOP)/device/sdksrc/xp/system/hal/driverWrapper/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/system/hal/interruptManager/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/system/sal/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/system/sal/bufferMgr/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/system/sal/dmaAllocator/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/system/sal/driverWrapper/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/system/mgmt/regMapper/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/system/init/urw/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/system/init/mre/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/system/init/isme/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/system/init/parser/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/allocator/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/interface/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/l2/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/l3/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/tunnel/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/multicast/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/acm/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/acl/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/mirror/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/debug/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/profileMgr/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/qos/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/nat/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/packetDriver/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/common/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/ipc/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/hwAccessMgr/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/linkManager/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/logger/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/sal/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/driverWrapper/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/interruptManager/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/age/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/init/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/of/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/xpDiag/debug/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/pl/blocks/include \
	-iquote $(REL_TOP)/utils/xp/Arbiter \
	-iquote $(REL_TOP)/utils/xp/ipc/cInterface/include \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/demo/config/include \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/demo/common/include \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/demo/xpsApp/include \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/demo/xpsApp/l2/include \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/demo/xpsApp/l3/include \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/demo/xpsApp/tunnel/include \
	-iquote $(REL_TOP)/platform/include \
	-iquote $(REL_TOP)/platform/svb/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/fl/debug/include \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/cInterface/fl/l2/include/ \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/cInterface/fl/allocator/include/ \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/cInterface/fl/l3/include/ \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/cInterface/fl/tunnel/include/ \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/cInterface/fl/multicast/include/ \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/cInterface/fl/nat/include/ \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/cInterface/fl/of/include/ \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/cInterface/fl/qos/include/ \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/cInterface/fl/acl/include/ \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/cInterface/fl/ipc/include/ \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/cInterface/fl/interface/include/ \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/cInterface/pl/include/ \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/openflow/include/ \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/acl/include/ \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/profiles/devices/xp80/urw/include/ \
        -iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/vif/include/ \
	-iquote $(REL_TOP)/device/sdksrc/xp/dm/devices/xp80/common/include/


INCLUDES.$(PRODUCT) += \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/include/packetTrakker       \
	-iquote $(REL_TOP)/device/sdksrc/xp/cInterface/packetTrakker/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/fl/packetTrakker/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/pl/include \
	-iquote $(REL_TOP)/device/sdksrc/xp/pl/managers/include \
	-iquote $(REL_TOP)/pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/include/packetTrakker/ \






ifdef TEST_WITH_HW
INCLUDES.$(PRODUCT) += -iquote $(REL_TOP)/device/sdksrc/xp/system/mgmt/os/linux/src/include 
else
INCLUDES.$(PRODUCT) += -iquote $(REL_TOP)/device/sdksrc/xp/system/mgmt/os/wmTest/src/include 
endif

################################################################################
# all the library sources and objects for this product
# only for use to generate shared SWIG library _<product>.so , at once
# (normally, we first build libs in each directory then combine them to the whole product)
################################################################################

LIB_SRCS.$(PRODUCT) =
$(foreach dir, $(SUBDIRS.$(PRODUCT)), $(eval LIB_SRCS.$(PRODUCT) += $$($(dir)/LIB_SRCS)))

LIB_OBJS.$(PRODUCT) =
$(foreach dir, $(SUBDIRS.$(PRODUCT)), $(eval LIB_OBJS.$(PRODUCT) += $$($(dir)/LIB_OBJS)))

