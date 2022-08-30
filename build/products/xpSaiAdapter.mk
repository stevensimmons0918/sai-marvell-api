########################################
# this $(PRODUCT).mk file has stuff specific to $(PRODUCT)
########################################

PRODUCT := $(FILENAME_BASE)


$(PRODUCT): \
sdklibs $(if $(SHARED),$(DIST)/libXpSaiAdapter.so ) \
$(if $(STATIC),$(DIST)/libXpSaiAdapter.a )
  

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
	$(REL_TOP)/xps \
	$(REL_TOP)/cpssHal \
	$(REL_TOP)/utils \
	$(REL_TOP)/$(SAI_SRC)/xpSai \
    $(REL_TOP)/$(SAI_SRC)/xpSai/include \
	$(REL_TOP)/$(SAI_SRC)/xpSai/util


# to build the total libsdk, we don't want archive directories first, since there is no way to archive archives
# instead, we collect all individual files
# and archive them

ifneq '$(STATIC)' ''
$(DIST)/libXpSaiAdapter.a: $(call OBJS_FROM_PROD_DIRS_CC_SRCS,LIB_SRCS) \
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
$(DIST)/libXpSaiAdapter.so: $(call OBJS_FROM_PROD_DIRS_CC_SRCS,LIB_SRCS) \
								$(DIST)/libXdkXps.so \
								$$(MKFS) | $$(@D)/.
	$(QUIET)$(ECHO) $(CXX) $@
	$(QUIET)$(CXX) -g -fPIC -rdynamic $(filter %.o, $^) ${LDFLAGS} -shared -o $@ \
								$(DIST)/libXdkXps.a \
								-L $(DIST) \
								-lpython2.7
	@$(ECHO)
	@$(ECHO)
	@$(ECHO) "$(notdir $@) built succesfully"
	@$(ECHO)
	@$(ECHO)

endif
########################
# sonic sai library
########################
CPSS_SAI_LIBS = $(DIST)/$(CPSS_ENABLER_LIB) $(CPSS_LIBS)
CPSS_SAI_LIBS += $(DIST)/$(CPSS_VERSION_LIB)

$(DIST)/libsai.so: $(call OBJS_FROM_PROD_DIRS_CC_SRCS,LIB_SRCS) \
								$(DIST)/libXpSaiAdapter.a \
								$$(MKFS) | $$(@D)/.
	
	$(CC) -g -fPIC -rdynamic $(filter %.o, $^) ${LDFLAGS}  -shared -o $@ \
									-Wl,--whole-archive \
	    							$(CPSS_SAI_LIBS) \
								$(DIST)/libXpPy.a	\
								-L $(DIST) -lXdkCpss -lcrypt \
								-lpython2.7  $(SHAREDLIBSOPTS)\
								-Wl,--no-whole-archive
	@$(OBJCOPY) --only-keep-debug $@ $@.dbg
	#@$(STRIP) --strip-debug --strip-unneeded $@
	@$(STRIP) -g $@
	@$(OBJCOPY) --add-gnu-debuglink=$@.dbg $@
	@$(ECHO)
	@$(ECHO)
	@$(ECHO) "$(notdir $@) built succesfully"
	@$(ECHO)
	@$(ECHO)

