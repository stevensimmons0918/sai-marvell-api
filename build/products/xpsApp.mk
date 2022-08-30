########################################
# this $(PRODUCT).mk file has stuff specific to $(PRODUCT)
########################################

PRODUCT := $(FILENAME_BASE)


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
################################################################################
SUBDIRS.$(PRODUCT) := \
	$(REL_TOP)/demo/common \
	$(REL_TOP)/demo/common/py \
	$(REL_TOP)/demo/config \
	$(REL_TOP)/demo/xpsApp \
	$(REL_TOP)/utils 

$(PRODUCT):  cpss sdklibs applibs xpPylibs $(DIST)/xpsApp


#sdk built by products/sdk.mk
