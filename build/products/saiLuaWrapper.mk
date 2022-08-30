########################################
# this $(PRODUCT).mk file has stuff specific to $(PRODUCT)
########################################

PRODUCT := $(FILENAME_BASE)


$(PRODUCT): $(DIST)/libsaiLuaWrapper.so $(DIST)/libsaiLuaWrapper.a
  

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
SUBDIRS.$(PRODUCT) += \
	$(REL_TOP)/$(SAI_SRC)/saiLuaWrapper


# to build the total libsdk, we don't want archive directories first, since there is no way to archive archives
# instead, we collect all individual files
# and archive them

ifneq '$(STATIC)' ''
$(DIST)/libsaiLuaWrapper.a: $(call OBJS_FROM_PROD_DIRS_CC_SRCS,LIB_SRCS) \
								$$(MKFS) | $$(@D)/.
	$(QUIET)$(ECHO) $(AR) $@
	$(QUIET)$(AR) $(ARFLAGS) $@ $(filter %.o, $^)
	@$(ECHO) 
	@$(ECHO)
	@$(ECHO) "$(notdir $@) built succesfully"
	@$(ECHO)  
	@$(ECHO)
endif

ifneq '$(SHARED)' ''
$(DIST)/libsaiLuaWrapper.so: $(call OBJS_FROM_PROD_DIRS_CC_SRCS,LIB_SRCS) \
								$$(MKFS) | $$(@D)/.
	$(QUIET)$(ECHO) $(CXX) $@
	$(QUIET)$(CXX) -g -fPIC -rdynamic $(filter %.o, $^) ${LDFLAGS} -shared -o $@ \
								-L $(DIST) \
								-lpython2.7
	@$(ECHO)
	@$(ECHO)
	@$(ECHO) "$(notdir $@) built succesfully"
	@$(ECHO)
	@$(ECHO)

endif


