########################################
# this $(PRODUCT).mk file has stuff specific to $(PRODUCT)
########################################

PRODUCT := $(FILENAME_BASE)
PREPROCESSING_V12 := (cd $(REL_TOP)/plugins/sai/sai-v1.2/SAI/saiShell/genScripts && ./preprocessing.sh $(KXDKFLAG))

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
# we have variables like        $(WM)/sde/mre/FOOBAR
# and targets like      $(WM)/sde/mre/foobar
################################################################################

$(PRODUCT):
	$(PREPROCESSING_V12)
	@$(ECHO)
	@$(ECHO)
	@$(ECHO) "preprocessing.sh for SAI 1.2 completed succesfully"
	@$(ECHO)
	@$(ECHO)


