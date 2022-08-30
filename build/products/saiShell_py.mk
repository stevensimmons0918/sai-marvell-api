########################################
# this $(PRODUCT).mk file has stuff specific to $(PRODUCT)
########################################

PRODUCT := $(FILENAME_BASE)

$(PRODUCT): $(DIST)/saiShell_py $(DIST)/libsaiShell.a $(DIST)/_saiShell.so $(DIST)/saiShell.py


################################################################################
# we include dir.mk files in these subdirectories
# they recursively include dir.mk files below them
#
# if this product depends on another product(s) to build
# do not list their directories that are not otherwise needed for us 
#
# in <subdir>/dir.mk files are directory-specific variables and make targets, prepended with <subdir>/
# for example:
# in $(WM)/sde/mre 
# we have variables like	$(WM)/sde/mre/FOOBAR
# and targets like	$(WM)/sde/mre/foobar
#
# 
# for this Python interface, include all directories that build the product that you interface to
# 
# the Python-C++ bridge code is generated automatically so you don't need to include any directory for that
################################################################################
SUBDIRS.$(PRODUCT) := 
SUBDIRS.$(PRODUCT) += \
	$(REL_TOP)/$(SAI_SRC)/saiShell/genScripts/saiShellCWrappers \
	$(REL_TOP)/build/SWIG


SINGLE_SRCS.$(PRODUCT) := \
	$(REL_TOP)/build/SWIG/prod_ifs/saiShell.i

################################################################################
# compiler options to preprocess all front-end files
################################################################################
INCLUDES.$(PRODUCT) := \
	-iquote $(REL_TOP)/xps/include \
	-iquote $(REL_TOP)/utils/state/include \
	-iquote $(REL_TOP)/utils/allocator/include \
	-iquote $(REL_TOP)/utils/sal/include  \
	-iquote $(REL_TOP)/demo/config/include \
	-iquote $(REL_TOP)/demo/common/include \
	-iquote $(REL_TOP)/demo/xpApp/include \
	-iquote $(REL_TOP)/demo/xpApp/l2/include \
	-iquote $(REL_TOP)/demo/xpApp/l3/include \
	-iquote $(REL_TOP)/demo/xpApp/tunnel/include \
    -iquote $(REL_TOP)/$(SAI_SRC)/xpSai/sai \
    -iquote $(REL_TOP)/$(SAI_SRC)/xpSai/sai/inc/ \
    -iquote $(REL_TOP)/$(SAI_SRC)/xpSai/include \
    -iquote $(REL_TOP)/$(SAI_SRC)/xpSai/util/include \
    -iquote $(REL_TOP)/$(SAI_SRC)/saiShell/genScripts/saiShellCWrappers/include \
    -iquote $(REL_TOP)/cpssHal/cpssHalDeviceMgr/include \
    -iquote $(REL_TOP)/cpssHal/cpssHalInit/include \
	-I$(REL_TOP)/cpss/mainPpDrv/h/cpss/dxCh/dxChxGen/pcl/ \
	-I$(REL_TOP)/cpss/mainPpDrv/h/cpss/dxCh/dxChxGen/virtualTcam/ \
	-I$(REL_TOP)/cpss/mainPpDrv/h/cpss/dxCh/dxChxGen/cnc/ \
    -I $(REL_TOP)/$(SAI_SRC)/xpSai/sai \
    -I $(REL_TOP)/$(SAI_SRC)/xpSai/sai/inc/ \
	$(CPSS_INCLUDES)

################################################################################
# all the library sources and objects for this product
# only for use to generate shared SWIG library _<product>.so , at once
# (normally, we first build libs in each directory then combine them to the whole product)
################################################################################
$(DIST)/libsaiShell.a: $(call OBJS_FROM_PROD_DIRS_CC_SRCS,LIB_SRCS) \
								$$(MKFS) | $$(@D)/.
	$(QUIET)$(ECHO) $(AR) $@
	$(QUIET)$(AR) $(ARFLAGS) $@ $(filter %.o, $^)
	@$(ECHO)
	@$(ECHO)
	@$(ECHO) "$(notdir $@) built succesfully"
	@$(ECHO)
	@$(ECHO)


LIB_SRCS.$(PRODUCT) =
$(foreach dir, $(SUBDIRS.$(PRODUCT)), $(eval LIB_SRCS.$(PRODUCT) += $$($(dir)/LIB_SRCS)))

LIB_OBJS.$(PRODUCT) =
$(foreach dir, $(SUBDIRS.$(PRODUCT)), $(eval LIB_OBJS.$(PRODUCT) += $$($(dir)/LIB_OBJS)))

