########################################
# this $(PRODUCT).mk file has stuff specific to $(PRODUCT)
########################################

PRODUCT := $(FILENAME_BASE)
KXDKFLAG := --kernel
PREPROCESSING_SAISHELL := (cd $(REL_TOP)/plugins/sai/SAI/saiShell/genScripts && ./preprocessing.sh $(KXDKFLAG))

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
	$(PREPROCESSING_SAISHELL)
	@$(ECHO)
	@$(ECHO)
	@$(ECHO) "preprocessing.sh for SAI completed succesfully"
	@$(ECHO)
	@$(ECHO)


