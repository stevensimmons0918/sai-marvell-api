########################################
# this $(PRODUCT).mk file has stuff specific to $(PRODUCT)
########################################

PRODUCT := $(FILENAME_BASE)

$(PRODUCT): $(DIST)/openXps_py $(DIST)/_openXps.so $(DIST)/openXps.py

SUBDIRS.$(PRODUCT) := $(SUBDIRS.openXps) 

INCLUDES.$(PRODUCT) :=  $(INCLUDES.sdk_py)

SINGLE_SRCS.$(PRODUCT) := \
	$(REL_TOP)/build/SWIG/prod_ifs/openXps.i

################################################################################
# all the library sources and objects for this product
# only for use to generate shared SWIG library _<product>.so , at once
# (normally, we first build libs in each directory then combine them to the whole product)
################################################################################
LIB_SRCS.$(PRODUCT) =
$(foreach dir, $(SUBDIRS.$(PRODUCT)), $(eval LIB_SRCS.$(PRODUCT) += $$($(dir)/LIB_SRCS)))

LIB_OBJS.$(PRODUCT) =
$(foreach dir, $(SUBDIRS.$(PRODUCT)), $(eval LIB_OBJS.$(PRODUCT) += $$($(dir)/LIB_OBJS)))

#LDFLAGS := -lpcap $(LIBGCOV) -lutil -lpython2.7 -lpthread -lxml2

ifndef MAC
#LDFLAGS += -lsensors -lrt -rdynamic
endif

################################################################################
$(DIST)/_openXps.so: $$(OBJSDIR)/SWIG/openXps_wrap.o  $$(LIB_OBJS.openXps_py) openXps $$(MKFS) | $$(@D)/.
	$(CXX) -iquote. $(GCXXFLAGS) -fPIC -I$(PYTHONINC) \
		$(filter %_wrap.o, $^) $(filter objs/native/../device/sdksrc/xp/xpDiag%.o, $^) \
		${LDFLAGS} -L$(DIST) -lOpenXps $(SHAREDFLAGS) -shared -Wl,-soname,_openXps.so -o $@
