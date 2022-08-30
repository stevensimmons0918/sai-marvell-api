########################################
# this $(PRODUCT).mk file has stuff specific to $(PRODUCT)
########################################

PRODUCT := $(FILENAME_BASE)

$(PRODUCT): $(DIST)/sdk_py $(DIST)/_sdk.so $(DIST)/sdk.py


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
# the Python-C++ bridge code is generated automatically so you don’t need to include any directory for that
################################################################################
SUBDIRS.$(PRODUCT) := \
	$(REL_TOP)/demo/common \
	$(REL_TOP)/demo/common/py \
	$(REL_TOP)/utils \
	$(REL_TOP)/xps \
	$(REL_TOP)/build/SWIG 



SINGLE_SRCS.$(PRODUCT) := \
	$(REL_TOP)/build/SWIG/prod_ifs/sdk.i

################################################################################
# compiler options to preprocess all front-end files
################################################################################
INCLUDES.$(PRODUCT) := \
	-iquote $(REL_TOP)/xps/include 				\
	-iquote $(REL_TOP)/utils/state/include 			\
	-iquote $(REL_TOP)/utils/allocator/include \
	-iquote $(REL_TOP)/utils/sal/include \
	-iquote $(REL_TOP)/utils/xps/include \
	-iquote $(REL_TOP)/demo/config/include \
	-iquote $(REL_TOP)/demo/common/include \
	-iquote $(REL_TOP)/demo/xpApp/include \
	-iquote $(REL_TOP)/demo/xpApp/l2/include \
	-iquote $(REL_TOP)/demo/xpApp/l3/include \
	-iquote $(REL_TOP)/demo/xpApp/tunnel/include \
	$(CPSS_INCLUDES)


################################################################################
# all the library sources and objects for this product
# only for use to generate shared SWIG library _<product>.so , at once
# (normally, we first build libs in each directory then combine them to the whole product)
################################################################################

LIB_SRCS.$(PRODUCT) =
$(foreach dir, $(SUBDIRS.$(PRODUCT)), $(eval LIB_SRCS.$(PRODUCT) += $$($(dir)/LIB_SRCS)))

LIB_OBJS.$(PRODUCT) =
$(foreach dir, $(SUBDIRS.$(PRODUCT)), $(eval LIB_OBJS.$(PRODUCT) += $$($(dir)/LIB_OBJS)))

