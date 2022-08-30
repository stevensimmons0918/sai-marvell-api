################################################################################
# bookkeeping at the bottom of each dir.mk
# and other calculations that can be put at the bottom
################################################################################

ifneq '$(GEN)' ''
-include $(D)/*_xp.mk
endif


################################################################################
# $D/SRCS understood by the compiler toolchain
################################################################################
$(D)/CC_SRCS := $(call CC_SRCS,$($(D)/SRCS))

################################################################################
# objects from sources listed in $D
# 
# we currently do not support objects in the top directory, so
# OBJS is the totality of all objects (not the objects in the top directory, with empty prefix)
################################################################################
$(D)/OBJS := $(call OBJS_FROM_CC_SRCS,$($(D)/CC_SRCS))
OBJS += $($(D)/OBJS)

# LIB_OBJS is the same, but only from LIB_SRCS
$(D)/LIB_OBJS := $(call OBJS_FROM_CC_SRCS,$($(D)/LIB_SRCS))


################################################################################
# directory prefix
################################################################################
DIR_PREFIX := $(D)$(if $(D),/)


################################################################################
# $(OTH_FILELIST) here is the source files found in 
# $(DIR_PREFIX)OTHER_SYSTEMS - self contained other make systems that we use
#
# we look for files with $(CC_XT) extensions
# but exclude files with two dots, as these are probably generated
#
# we have to make the other systems first, as they might be generating source files that we look for, during the build
#
# we use "find" to look for source files, and change this file only if the list of found files is different 
# that is done with CHANGE_TARGET_ONLY_IF_OUT_DIFF found in variables.mk
################################################################################

OTH_FILELIST := $(DIR_PREFIX)oth_filelist.rel.txt

$(OTH_FILELIST): $(addsuffix /make, $($(DIR_PREFIX)OTHER_SYSTEMS))
	find $($(DIR_PREFIX)OTHER_SYSTEMS) \
		\( -name $(firstword $(CC_XT)) \
		$(foreach xt, $(wordlist 2, $(words $(CC_XT)), $(CC_XT)), -o -name '*.$(xt)') \) \
		! -name '*.*.*' \
	$(CHANGE_TARGET_ONLY_IF_OUT_DIFF)


################################################################################
# $(DIR_PREFIX)oth_filelist.customer.txt , $(OTH_FILELIST) here is all the source control files found in
# $(DIR_PREFIX)OTHER_SYSTEMS - self contained other make systems that we use
################################################################################
$(DIR_PREFIX)oth_filelist.customer.txt: phony
	$(call SRC_CTRL_FILES_IN, $($(DIR_PREFIX)OTHER_SYSTEMS))  $(CHANGE_TARGET_ONLY_IF_OUT_DIFF)


################################################################################
# $(DIR_PREFIX)filelist.customer.txt is source files needed to build products for customer:
# mentioned in this directory make file fragment as $D/SRCS
# and its recursive include children, 
# and files that can be deduced as necessary from the above, 
# all the source control files in black boxes: $(DIR_PREFIX)oth_filelist.customer.txt 
# and, for top directory only with $(DIR_PREFIX) empty, we add $(SINGLE_SRCS) combined from all the products
# $(SINGLE_SRCS.$(PRODUCT))
# 
# that includes generated C/C++ files that are meant to be read by the customer, no other generated files
# 
# files listed as relative paths, one on a line, without repetitions
#
# for "deduced" files, these are currently just header files:
# we simply consider all words in our .d dependency files (which we have to build first if needed)
#
# we use COMBINE_SRCS_TO_FILELISTS defined in variables.mk to join sources here, and deduced header files
# with the contents of filelist.txt files below us, if any, on which we depend
################################################################################
$(DIR_PREFIX)SUBDIR_FILELISTS.CUSTOMER := \
	$($(DIR_PREFIX)SUBDIRS:=/filelist.customer.txt)$(if $($(DIR_PREFIX)OTHER_SYSTEMS), $(DIR_PREFIX)oth_filelist.customer.txt)

$(eval $(call COMBINE_SRCS_TO_FILELISTS, \
	$(DIR_PREFIX)filelist.customer.txt, $($(DIR_PREFIX)SUBDIR_FILELISTS.CUSTOMER), $($(DIR_PREFIX)SINGLE_SRCS) $($(DIR_PREFIX)SRCS)))


################################################################################
# filelist.rel.txt is a file like filelist.customer.txt, but only C/C++ sources
# 
# filelist.txt is the same, but absolute paths
################################################################################

$(DIR_PREFIX)SUBDIR_FILELISTS.REL := \
	$($(DIR_PREFIX)SUBDIRS:=/filelist.rel.txt)$(if $($(DIR_PREFIX)OTHER_SYSTEMS), $(DIR_PREFIX)oth_filelist.rel.txt)

$(eval $(call COMBINE_SRCS_TO_FILELISTS, \
	$(DIR_PREFIX)filelist.rel.txt, $($(DIR_PREFIX)SUBDIR_FILELISTS.REL), $($(DIR_PREFIX)SINGLE_SRCS) $($(DIR_PREFIX)CC_SRCS)))

# canned recipe to make absolute filelist from relative
define MAKE_ABS_FILELIST
cat $< | \
	while read line; do \
		echo $(CURDIR)/$$line; \
	done \
	> $@
endef

$(DIR_PREFIX)filelist%.txt: $(DIR_PREFIX)filelist%.rel.txt dir_bottom.mk
	$(MAKE_ABS_FILELIST)

$(DIR_PREFIX)filelist.txt: $(DIR_PREFIX)filelist.rel.txt dir_bottom.mk
	$(MAKE_ABS_FILELIST)


################################################################################
# make so that you can evaluate variables correctly from within recipes
# inside recipes is deferred evaluation, so we store the current value of such a variable in a 
# target-specific variable
################################################################################

################################################################################
# make $(TARGET_VARS) variables accessible to anybody
#
# notice how we use the GNU Make 3.82 preference to shorter stems for implicit rules
# so that we catch the deepest directory rule for any target
################################################################################
$(foreach var,$(TARGET_VARS),$(eval \
	$(DIR_PREFIX)% $(if $(DIR_PREFIX),$(OBJSDIR)/$(DIR_PREFIX)%): $(var):= $($(var))))

# CXX variable accessible to compile *.o targets
# equal to value of $(D)/CXX only if that overrides CXX for this directory
$(OBJSDIR)/$(DIR_PREFIX)%.o: CXX := $(if $($(D)/CXX), $($(D)/CXX), $(CXX))

# same thing for AR for *.a targets
$(OBJSDIR)/$(DIR_PREFIX)%.a: AR := $(if $($(D)/AR), $($(D)/AR), $(AR))

# and a few more for filelist.txt
$(DIR_PREFIX)filelist.txt: SUBDIR_FILELISTS:= $(SUBDIR_FILELISTS)
$(DIR_PREFIX)filelist.txt: DIR_PREFIX:= $(DIR_PREFIX)
$(DIR_PREFIX)filelist.txt: DIR_OBJS:= $(DIR_OBJS)

# and a few more for ...filelist.....txt
$(DIR_PREFIX)filelist.%: SUBDIR_FILELISTS:= $(SUBDIR_FILELISTS)
$(DIR_PREFIX)filelist.%: SUBDIR_FILELISTS.REL:= $(SUBDIR_FILELISTS.REL)
$(DIR_PREFIX)filelist.%: DIR_PREFIX:= $(DIR_PREFIX)
$(DIR_PREFIX)filelist.%: DIR_OBJS:= $(DIR_OBJS)
$(DIR_PREFIX)oth_filelist.%: DIR_PREFIX:= $(DIR_PREFIX)


################################################################################
#establish the above targets for other systems called from this directory $D
################################################################################
$(foreach dir, $($(DIR_PREFIX)OTHER_SYSTEMS), $(eval \
	$(call PHONY_MAKE_TARGET, $(dir)) \
))


################################################################################
# low-priority (long stem) pattern rule to catch anything in and below $D that was not caught by anything else
# and redirect to the $(OBJSDIR) directory
# but make sure the directory itself is done correctly
################################################################################
$(D)/.:
	$(QUIET)$(MKDIR) -p $@
$(D)/%: $(OBJSDIR)/$(D)/% ;


################################################################################
# generate explicit rules to depend object of generated library source on the source
# generated sources may not have a dependency makefile .d yet so we have to explicitly do this
################################################################################
$(foreach src, $($(D)/GEN_LIB_SRCS), $(eval $(OBJSDIR)/$(subst cpp,o,$(src)): $(src)))


#bookkeeping of the directory stack
D := $(DIR$(DEPTH))
DEPTH := $(patsubst %1,%,$(DEPTH))
P := $(DIR$(DEPTH))
