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
#$(PRODUCT)_IF := \
#	$(HS)/include \
#	system/mgmt \
#	pl \
#	fl \

#$(PRODUCT)_IF.TAR := $(DIST)/xpliant_sdk_prelim_release_1_00.tar 


################################################################################
# the whole product
################################################################################
$(PRODUCT): \
$(if $(SHARED),$(DIST)/libXdk.so ) \
$(if $(STATIC),$(DIST)/libXdk.a ) \

#	$($(PRODUCT)_IF.TAR)
  # all built here


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

# to build the total libsdk, we don't want archive directories first, since there is no way to archive archives
# instead, we collect all individual files
# and archive them

ifneq '$(STATIC)' ''
$(DIST)/libXdk.a: $(call OBJS_FROM_PROD_DIRS_CC_SRCS,LIB_SRCS) $$(MKFS) | $$(@D)/.
	$(MAKE_STATIC_LIB)
endif


ifneq '$(SHARED)' ''
$(DIST)/libXdk.so: $(call OBJS_FROM_PROD_DIRS_CC_SRCS,LIB_SRCS) $$(MKFS) | $$(@D)/.
	$(MAKE_SHARED_LIB)
	@$(ECHO)
	@$(ECHO) "$(notdir $@) built succesfully"
	@$(ECHO)

endif
