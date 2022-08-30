########################################
# this $(PRODUCT).mk file has stuff specific to $(PRODUCT)
########################################

PRODUCT := $(FILENAME_BASE)

########################################
# this product is a collection of partial libraries of sdk
# 
# here list the basenames (without extensions) of these libraries
########################################

LIB_BASENAMES := \
    libXdkXps \
	libXpsState \
	libXpsAllocator \
	libXpsSal 

# if SHARED is defined, make all $(DIST)/...so
# if STATIC is defined, make all $(DIST)/...a
$(PRODUCT): \
$(if $(SHARED),$(patsubst %,$(DIST)/%.so ,$(LIB_BASENAMES))) \
$(if $(STATIC),$(patsubst %,$(DIST)/%.a ,$(LIB_BASENAMES))) \

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
	$(REL_TOP)/utils 


SUBDIRS.$(PRODUCT) += \
	$(REL_TOP)/xps \
	$(REL_TOP)/cpssHal

# Targets that build granular libraries (either static libs or shared objects)
ifneq '$(STATIC)' ''
$(DIST)/libXdkXps.a: $(call OBJS_FROM_PROD_DIRS_CC_SRCS,LIB_XDK_XPS_SRCS) $$(MKFS) | $$(@D)/.
	$(MAKE_STATIC_LIB)
$(DIST)/libXpsState.a: $(call OBJS_FROM_PROD_DIRS_CC_SRCS,LIB_XPS_STATE) $$(MKFS) | $$(@D)/.
	$(MAKE_STATIC_LIB)
$(DIST)/libXpsAllocator.a: $(call OBJS_FROM_PROD_DIRS_CC_SRCS,LIB_XPS_ALLOCATOR) $$(MKFS) | $$(@D)/.
	$(MAKE_STATIC_LIB)
$(DIST)/libXpsSal.a: $(call OBJS_FROM_PROD_DIRS_CC_SRCS,LIB_XPS_SAL) $$(MKFS) | $$(@D)/.
	$(MAKE_STATIC_LIB)
endif


ifneq '$(SHARED)' ''
$(DIST)/libXdkXps.so: $(call OBJS_FROM_PROD_DIRS_CC_SRCS,LIB_XDK_XPS_SRCS) $$(MKFS) | $$(@D)/.
	$(MAKE_SHARED_LIB)
$(DIST)/libXpsState.so: $(call OBJS_FROM_PROD_DIRS_CC_SRCS,LIB_XPS_STATE) $$(MKFS) | $$(@D)/.
	$(MAKE_STATIC_LIB)
$(DIST)/libXpsAllocator.so: $(call OBJS_FROM_PROD_DIRS_CC_SRCS,LIB_XPS_ALLOCATOR) $$(MKFS) | $$(@D)/.
	$(MAKE_STATIC_LIB)
$(DIST)/libXpsSal.so: $(call OBJS_FROM_PROD_DIRS_CC_SRCS,LIB_XPS_SAL) $$(MKFS) | $$(@D)/.
	$(MAKE_STATIC_LIB)
endif
