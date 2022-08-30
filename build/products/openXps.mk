########################################
# this $(PRODUCT).mk file has stuff specific to $(PRODUCT)
########################################

PRODUCT := $(FILENAME_BASE)

################################################################################
# interface to this product
# 
# header files below the following directories, used in this product, are in the interface
# they are bundled to the following .tar file
#
# you must use these variable names for the directories and the .tar file
################################################################################

LIBNAME := OpenXps

################################################################################
# the whole product
################################################################################
$(PRODUCT): \
$(if $(SHARED),$(DIST)/lib$(LIBNAME).so.0 )

################################################################################
# we include dir.mk files in these subdirectories
# they recursively include their dir.mk files below them
#
# in <subdir>/dir.mk files are directory-specific variables and make targets, prepended with <subdir>/
# for example:
# in $(WM)/sde/mre 
# we have variables like	$(WM)/sde/mre/FOOBAR
# and targets like	$(WM)/sde/mre/foobar
################################################################################
SUBDIRS.$(PRODUCT) := \
	$(REL_TOP)/xps \
	$(REL_TOP)/demo/common/py \

#LDFLAGS := -lpcap $(LIBGCOV) -lutil -lpython2.7 -lpthread -lxml2

ifndef MAC
#LDFLAGS += -lsensors -lrt -rdynamic
endif

ifneq '$(SHARED)' ''
$(DIST)/lib$(LIBNAME).so.0: $(call OBJS_FROM_PROD_DIRS_CC_SRCS,LIB_SRCS) $$(MKFS) | $$(@D)/.
	$(CXX) -fPIC $(filter %.o, $^) $(filter %.a, $^) $(filter %.a, $^) ${LDFLAGS} -shared -Wl,-soname,lib$(LIBNAME).so.0 -o $@ ; \
	$(shell ln -sf lib$(LIBNAME).so.0 $(DIST)/lib$(LIBNAME).so)
	@$(ECHO)
	@$(ECHO) "$(notdir $@) built succesfully"
	@$(ECHO)
endif
