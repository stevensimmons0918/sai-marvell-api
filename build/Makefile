################################################################################
# Makefile for the xp80
#
# Please consult with the Software Team before making any changes
################################################################################

.SUFFIXES:

include variables.mk


# all objects
OBJS       :=


# all these products have products/<product>.mk specific makefile fragment
# these fragments are included in the order given, so the order may matter
# for example, if sources SUBDIRS.foobar for product foobar, listed in products/foobar.mk, include
# sources for foo and bar,
# then foobar should be listed after foo and bar
MOD_PRODUCTS :=   sdklibs xpPylibs  xpSaiApp xpSaiAdapter sdk xpsApp applibs openXps mrvlHal saiPreproc
ifndef MAC
MOD_PRODUCTS += saiShell_py xpsApp_py sdk_py openXps_py saiLuaWrapper
endif

# Control the verbose
ifndef VERBOSE
	QUIET := @
endif

MOD_PRODUCTS += sdkPlatform

MOD_PRODUCTS += cpss

GCCVERSION = $(shell $(CC) -dumpversion)
GCCVERSION_WERROR = $(shell expr `$(CC) -dumpversion | cut -f1 -d.` \>= 5)

.PHONY: $(MOD_PRODUCTS)

.PHONY: all
all: ; $(info $$var is [${var}])echo Hello world

print-%: ; @echo $*=$($*)

################################################################################
# >make
# builds $(DEFAULT_GOALS)
################################################################################
DEFAULT_GOALS := $(DEFAULT_PRODUCTS) cscope filelist.txt

A:= $(shell echo $(DEFAULT_GOALS))
$(info $(A))
# goals that need additional makefiles
USER_MAKECMDGOALS := $(if $(MAKECMDGOALS), $(MAKECMDGOALS), $(DEFAULT_GOALS))
ADD_MAKEFILE_GOALS := $(filter-out clean %/clean help usage bogus%, $(USER_MAKECMDGOALS))

ifeq '$(MAKECMDGOALS)' ''
$(info checking default goals: $(DEFAULT_GOALS))
endif

.PHONY : all
all : $(DEFAULT_GOALS)


# to force update, depend on phony
.PHONY: phony


# this is so we can use things like $$(@D) in prerequisites
.SECONDEXPANSION:

CSCOPE_PRODUCTS := $(addprefix cscope., $(MOD_PRODUCTS))

.PHONY: cscope $(CSCOPE_PRODUCTS)
cscope : cscope.out

cscope.out: filelist.txt Makefile
	cscope -b -q -k -i filelist.txt

$(CSCOPE_PRODUCTS): cscope.%: cscope/%/cscope.out

cscope/%/cscope.out: filelist.%.txt Makefile| $$(@D)/.
	cd $(@D); \
	cscope -b -q -k -i $(CURDIR)/$<
cscope.in: FORCE
	cd .. ; find . -type f |grep -e '\.h$$' -e '\.hpp$$' -e '\.cpp$$' | grep -v xp70 |grep -v xp80 |grep -v ./dist >cscope.in
FORCE:


ifneq '$(STATIC)' ''
################################################################################
# default pattern rule to build *.a
#
# $(OBJSDIR)/subdir/libfoobar.a
# builds from all the sources listed in $(subdir/SRCS)
################################################################################
.PRECIOUS: $(OBJSDIR)/%.a
$(OBJSDIR)/%.a: $$($$(patsubst $$(OBJSDIR)/$$(PRC),$$(PRC),$$(@D))/LIB_OBJS) $$(MKFS) | $$(@D)/.
	$(MAKE_STATIC_LIB)

endif


################################################################################
# default pattern rule to build *.o
# (it is faster to use your own rather than GNU Make default)
#
# generate dependencies too - they include the dependency on the source, so we don't need it here
#
# $(D) is target-specific variable defined in dir_bottom.mk
# set to relative directory to the makefile fragment dir.mk that compiles this target
#
# we are assuming any source file may need any generated header
# generated headers are in make targets $(INCL_GENERATED_CONTENTS),
# or a side effect of those ,
# so we make that first before attempting to compile the source file here
#
# dummy is there to prevent applying this rule in preference to more specific rules with implicit prerequisites that need to be
# built
################################################################################
.PRECIOUS: $(OBJSDIR)/%.o
$(OBJSDIR)/%.o: $$(MKFS) $$(INCL_GENERATED_CONTENTS) $(OBJSDIR)/%.dummy | 	$$(@D)/.
	@$(ECHO) $(CXX) $@
	$(QUIET)$(CXX) $($(D)/CXXFLAGS) -MMD -MP -c  $(wildcard $(patsubst $(OBJSDIR)/%,%, $(foreach xt, $(CC_XT), $(@:o=$(xt))))) \
	-o $@
	$(QUIET)mv $(@:o=d) $@.d

.PHONY: $(OBJSDIR)/%.dummy
$(OBJSDIR)/%.dummy: ;



################################################################################
# testing program for class Foobar.cpp compiled from FoobarTest.cpp
#
# these depend on libraries defined in each subdirectory dir.mk , other than that directory :
#	$$(D)/TEST_DEPEND_LIBS
################################################################################
%Test: %Test.cpp $$($$(D)/TEST_DEPEND_LIBS) $$($$(D)/LIB) $$(MKFS)
	$(QUIET)$(CXX) -DDBG $(filter-out %NDEBUG, $($(D)/CXXFLAGS)) $< $($(D)/LIB) $($(D)/TEST_DEPEND_LIBS) \
		-o $@


# make a directory
# directory targets are recognized with suffix /.
.PRECIOUS: %/.
%/.:
	$(QUIET)$(MKDIR) -p $(@:.=)


# initialize files in $(DIST) directory that are not built at the customer site, but distributed to them alone
DIST_NOT_BUILT :=


# if we are building a customer distribution, not all products might be there,
# if not there yet, we will copy them by hand in makedist
include $(wildcard $(patsubst %,products/%.mk, $(MOD_PRODUCTS)))


################################################################################
# rule to build interface
# $(product_IF.TAR)
# from header files in
# $(product_IF)
#
# product is $1
#
# also include all source directories for your product, from $(SUBDIRS.product) - they will be empty if they do not contain
# any header interfaces
#
# archive all the files in the .tar file, so they extract to the subdirectory "xdk"
################################################################################
define IF_RULE
$$($1_IF.TAR): filelist.rel.$1.txt products/$1.mk Makefile
	tar -cpvf $$@ --no-recursion --transform 's,^,xdk/,' \
		$$(filter $$(subst //,/,$$(addsuffix /$(PRC).h,$($1_IF))), $$(shell cat $$<)) \
		$$(sort $$(dir $$(shell git ls-files $(SUBDIRS.$1))))
endef

# establish the rules above
$(foreach prod, $(MOD_PRODUCTS), $(eval $(call IF_RULE,$(prod))))


###############################################################################
# we include dir.mk files in $SUBDIRS : subdirectories mentioned in the files products/*.mk
# they recursively include dir.mk files below them
################################################################################
SUBDIRS := $(sort $(foreach prod, $(MOD_PRODUCTS), $(SUBDIRS.$(prod))))

$(call INCL_SUBDIRS, $(SUBDIRS))

# after subdirectories are established, we can include some rules that depend on them
include $(wildcard $(patsubst %,products/%_rules.mk, $(MOD_PRODUCTS)))


################################################################################
# SINGLE_SRCS combined for all MOD_PRODUCTS
################################################################################
SINGLE_SRCS := $(sort $(foreach prod, $(MOD_PRODUCTS), $(SINGLE_SRCS.$(prod))))


# here this is not at the bottom, it is just to pick up some rules for use below
$(BOTTOM_MAGIC)


################################################################################
# establish various filelist.<product>.<type>.txt targets
# combine them from the analogous files in product directories below
# type is the argument to the function COMBINE_PRODUCT_FILELISTS_BELOW
# see the function comments in variables.mk for more explanation
################################################################################
$(eval $(call COMBINE_PRODUCT_FILELISTS_BELOW,rel,CC_SRCS))
$(eval $(call COMBINE_PRODUCT_FILELISTS_BELOW,customer))


# generated dependency makefiles
GEN_DEPS :=


################################################################################
# SWIG stuff below
################################################################################

################################################################################
# rules generated for each Python interface products/$1_py.mk
#
# these needed for files, which are not generated source files
################################################################################
define PY_DBG_RULES

################################################################################
# rule to make the file SWIG/$1.gen.cpp for a product $1_py
# this file is autogenerated as needed from all the .c(pp) files in the product
#
# the rule uses make_c_if.pl script with $(LIB_SRCS.$1) (suitably escaped for evaluation) files one by one
#
# we count this not as a generated source, because it is needed only as interface to SWIG and it
# repeats fragments of the .c(pp) source files
#
# because all the *.pm files are explicitly referenced from the files that need them
# (not through a wildcard)
# then it is OK to depend on the wildcard only, and not have to depend on the directory SWIG
# for additions or deletions of .pm files
################################################################################
SWIG/$1.gen.cpp: SWIG/make_new_c_if.pl $(wildcard SWIG/*.pm) \
	$$$$(patsubst %c, %i, $$$$(patsubst %pp, %, $$$$(filter-out genScripts/saiShellCWrappers, $$$$(LIB_SRCS.$1_py)))) \
	$$$$(patsubst %c, %i, $$$$(patsubst %pp, %, $$$$(filter genScripts/saiShellCWrappers, $$$$(LIB_SRCS.$1_py)))) \
	Makefile
	$(QUIET)echo -n > $$@
	$(QUIET)$$(foreach file, $$(filter-out ../pipeline-profiles/xpDefault/%.i, $$^), $$< $$(file:i=c) <$$(file) >>$$@; )
	$(QUIET)$$(foreach file, $$(filter ../pipeline-profiles/xpDefault/%.i, $$^), $$< $$(file:i=c) <$$(file) >>$$@; )


# compute and save some variables

#all the non-system %included files
# in SWIG/prod_ifs/$1.i
SWIG_INCL_DEPS_$1 := $(shell (awk \
	'{ if ($$1 == "%include" && $$2 ~ /^"/) print $$2}' |	\
	 sed 's/"//g') <SWIG/prod_ifs/$1.i)

# filter only generated files from above, signified by the suffix .gen.i
SWIG_GEN_IFS_$1 := $$(filter-out SWIG/%, $$(filter %.gen.i, $$(SWIG_INCL_DEPS_$1)))

# all the front end files for product $1
SWIG_FRONT_END_$1 := $$(subst gen.i,h, $$(SWIG_GEN_IFS_$1))

################################################################################
# list of front end header files, recovered from the modified .gen.i listing in the foobar.i file
################################################################################
SWIG/$1.front.txt: Makefile SWIG/prod_ifs/$1.i
	$(QUIET)echo -n > $$@
	$(QUIET)$$(foreach file, $$(SWIG_FRONT_END_$1), echo $$(file) >>$$@;)
endef

#establish the rules above for anybody who might need it
$(foreach prod, $(patsubst products/%_py.mk, %, $(wildcard products/*_py.mk)), $(eval PY_DBG += $(prod)))
$(foreach prod, $(PY_DBG), $(eval $(call PY_DBG_RULES,$(prod))))


ifneq '$(GEN)' ''

################################################################################
# foobar.gen.i interface for SWIG - modified file foobar.h that SWIG did not like
#
# we use the preprocessed foobar.i file to produce the target
################################################################################

# first delete any __attribute__(...) GCC keyword blocks, as SWIG does not like them, easy to do by sed
# then apply Perl for remaining stuff: make_c.pl and make_if.pl
# make_if.pl program depends on module(s) .pm in the same SWIG directory
# we do not need to depend on the SWIG directory itself, to account for deleting files
# because that would result in an edit to the .pl file anyway
%.gen.i: SWIG/make_if.pl $(wildcard SWIG/*.pm) SWIG/make_c.pl %.h.i Makefile
	$(QUIET)echo "SWIG/make_if.pl $@"
	$(QUIET)sed -e 's/__attribute__\(\s*(\s*\)\+\w*\(\s*)\s*\)\+//' <$*.h.i | \
	SWIG/make_c.pl | \
	SWIG/make_if.pl $*.h $@;

################################################################################
# rules generated for each Python interface products/$1_py.mk
#
# these needed only for files that are generated sources
################################################################################
define PY_DBG_GEN_RULES

GEN_DEPS += $(SWIG_FRONT_END_$1:=.i.d)

#$(DIST)/foobar.py depends on the includees from SWIG/foobar.i
$(DIST)/$1.py: $(SWIG_INCL_DEPS_$1)

################################################################################
# common generated files for a SWIG interface to a product
# foobar.gen.h has only C elements
# foobar.gen.i has C and SWIG elements
#
# note that the make implicit rule pattern stem length here is only 1, so should be less
# and take precedence over the previous rule for
# %gen.h
#
# the dep on SWIG/prod_ifs/$1.i is because that is where $(SWIG_GEN_IFS_$1) is calculated from
# our target depends not just on the contents of $(SWIG_GEN_IFS_$1)
# but on the value $(SWIG_GEN_IFS_$1) itself
################################################################################
SWIG/$1.gen.%: SWIG/make_common_gen.pl SWIG/prod_ifs/$1.i $(SWIG_GEN_IFS_$1) Makefile
	$(QUIET)$$< $$@ $(SWIG_GEN_IFS_$1)

WRAP_I := SWIG/$1_wrap.i
# to compile the SWIG wrapper, we first generate this preprocessed file, then we will modify it later before completing
# compilation
# here we establish the flags for the preprocessing
$$(WRAP_I): CPPFLAGS := -Wno-uninitialized $(INCLUDES.$1_py) -I$(PYTHONINC) -iquote.

# additional dependencies of *wrap.i
$$(WRAP_I): SWIG/prod_ifs/$1.i $(SWIG_FRONT_END_$1) SWIG/$1.gen.h


GEN_DEPS += $$(WRAP_I).d $(addsuffix .i.d, $(basename $(LIB_SRCS.$1_py)))

endef


# establish the rules above for files SWIG/prod_ifs/*.i
$(foreach prod, $(PY_DBG), $(eval $(call PY_DBG_GEN_RULES,$(prod))))


################################################################################
# SWIG Python intermediate files
################################################################################

.PRECIOUS: $(DIST)/%.py SWIG/%_wrap.cxx

################################################################################
# Python script that you import to Python, and it looks for shared _$*.so to load to Python
# swig command also generates $*_wrap.cxx - cpp wrapper file to Python
# not in the correct place so it needs to be moved
################################################################################
$(DIST)/%.py: SWIG/prod_ifs/%.i Makefile | $$(@D)/.
	swig -module $* -python -threads -c++ \
		-w454,325,$(SWIGWARN_PARSE_NESTED_CLASS),$(SWIGWARN_IGNORE_OPERATOR_EQ) \
		-shadow -outdir $(@D) $<

#move the wrapper to the correct place
# mv or swig will sometimes round timestamp on *_wrap.cxx to whole second,
# so we have to touch it
SWIG/%_wrap.cxx: $(DIST)/%.py Makefile
	mv SWIG/prod_ifs/$(notdir $@) $@
	touch $@
$(OBJSDIR) :
	mkdir -p $(OBJSDIR)
$(DIST) :
	mkdir -p $(DIST)
	ln -s $(DIST) dist
#$(OBJSDIR)/aapl : | $(OBJSDIR)
#	mkdir -p $(OBJSDIR)/aapl
#$(DIST)/aapl : | $(DIST)
#	mkdir -p $(DIST)/aapl
#$(DIST)/aapl/libAaplUtil.a : $(DIST)/libAAPL_noinst.a
endif #ifneq '$(GEN)' ''
A := $(shell echo $(OBJSDIR))
$(info $(A))


################################################################################
#shared library to load to Python that the SWIG interface calls upon
# built from all the wrapper, and all the normal objects of your product
################################################################################
$(DIST)/_%.so: $$(OBJSDIR)/SWIG/%_wrap.o $$(LIB_OBJS.$$*_py) $$(MKFS) | $$(@D)/.
ifdef UCLIBC
	$(QUIET)$(CXX) -iquote. $(GCXXFLAGS) $(INCLUDES) $(INCLUDES.$*_py) -fPIC -I$(PYTHONINC) \
		$(filter %.o, $^) $(filter %.a, $^) \
		-lpcap $(LIBGCOV) -rdynamic -lutil -lxml2 -lrt -lpython2.7  $(SHAREDFLAGS) -shared -o $@ ; \
		#ln -sf $(DIST)/_$*.so $(DIST)/lib$*.so
		cd $(DIST); ln -sf _$*.so lib$*.so
else
	$(QUIET)$(CXX) -iquote. $(GCXXFLAGS) $(INCLUDES) $(INCLUDES.$*_py) -fPIC -I$(PYTHONINC) \
		$(filter %.o, $^) $(filter %.a, $^) \
		$(LIBGCOV) -rdynamic -lpython2.7   $(SHAREDFLAGS) -shared -o $@
		#ln -sf $(DIST)/_$*.so $(DIST)/lib$*.so
		cd $(DIST); ln -sf _$*.so lib$*.so
endif

################################################################################
# wrapper compilation
#
# wrapper to compile does not have its own dir.mk, instead we use include search path flags defined in product_py.mk
#
# we preprocess the wrapper first, then modify that, and finish compile
################################################################################
.PRECIOUS: $$(OBJSDIR)/SWIG/%_wrap.o
$(OBJSDIR)/SWIG/%_wrap.o: SWIG/%_wrap_modified.cpp $$(MKFS) | $$(@D)/.
	$(QUIET)$(ECHO) $(CXX) $@
	$(QUIET)$(CXX) $(filter-out -Werror, $(GCXXFLAGS)) -Wno-uninitialized -fPIC -c $< -o $@

################################################################################
# modified preprocessed SWIG wrapper
#
# Let us explain the whole process that causes the need for this.
#
# for some functionality to wrap by SWIG, we would like to modify a front-end file foobar.h, and use such modified file for
# both %include and #include SWIG interface sections
#
# if a method argument is not understood by SWIG, we would use a typemap
# but if a declared symbol, in class/struct or global scope, is not understood, we have to use this technique
#
# we want to change the type of the symbol in foobar.h from, say, type0 which is not understood, to type1 which is
# we supply in SWIG interface additional C methods that Python uses to process type0, that are missing from type1
# the user can then call type0 functionality
#
# Of course, we cannot really change foobar.h, because that file unchanged, must be used for the wrappee.
# So, to "change" the file in the %include section, we automatically make the modification when we create the file foobar.gen.i
# which is the one %included.
#
# For the #include section, we also do not #include foobar.h directly, but instead, %include a top file, which then #includes all
# the front ends. Since it would be very difficult to parse the unprocessed foobar.h and make the modifications, we always have
# to preprocess first, and then we could modify and #include the preprocessed file.
#
# However, the preprocessed front-end header file will not only contain its own fragments, but also its includees, both user and
# system. We can either restrct the preprocessed file to itself only or not.  If we do, the files that are included in the front end
# but not front end themselves, including system files, will never be included in the wrapper and the compilation will bomb. If we
# don't then they may be included multiple times and the compilation will bomb.
#
# So we have to be more clever. We do not change the #included files in the %included top file, we let the wrapper .cxx file be
# with them, and we preprocess the wrapper first. Of course it can only be preprocessed but won't compile as explained above.
# Then we modify the preprocessed wrapper to make the required replacements, to match the ones already made earlier in
# %include section, and then we finish the compilation.
#
# the modification is done with SWIG/make_c.pl script and it is only performed in the preprocessed fragments coming from the
# front end files listed in SWIG/foobar.front.txt.  Otherwise, we would be replacing "not understood" symbols in system files.
################################################################################

.PRECIOUS: SWIG/%_wrap_modified.cpp
SWIG/%_wrap_modified.cpp: SWIG/make_c.pl SWIG/%_wrap.i SWIG/%.front.txt
	$< SWIG/$*.front.txt >$@ <SWIG/$*_wrap.i
	python SWIG/make_structArray.py $*

# generated top section file for the foobar.i SWIG interface
# Due to syntax error occurred in one of the .gen.i files for Octeon Thunderx,
# need to correct syntax error if any for the sdk_py target compilation.
# Verify all the .gen.i files for correct syntax by calling
# sdk_py_syntax_error_fix.sh script.
SWIG/%.top.i: SWIG/%.front.txt SWIG/%.gen.h Makefile
	echo %{ >$@
	echo '#include "SWIG/$*.gen.h"' >>$@
	awk '{print "#include \"" $$1 "\""}' <$< >>$@
	echo %} >>$@
ifeq (${CROSS_COMPILE},aarch64-thunderx-linux-gnu-)
	$(shell ./sdk_py_syntax_error_fix.sh)
endif


################################################################################
# starter script for Python interface to product $*
################################################################################
$(DIST)/%_py: | $$(@D)/.
	ln -sf $(abspath SWIG/prod_py) $@

#end of SWIG stuff


################################################################################
# pattern rule to build *.i - preprocessed file
# for debugging
# and for SWIG for .cxx and .h files
#
# generate dependencies too - they include the dependency on the source, so we don't need it here
#
# we don't want to clash path/foobar.i obtained from path/foobar.cpp as debugging
# with the one obtained during the SWIG build for path/foobar.h, so this one we call path/foobar.h.i
#
# preprocess of files path/foobar.h initially generates $(OBJSDIR)/path/foobar.d which is incorrect, as it has foobar.o, not foobar.i as
# target, so we fix it with sed
# at the same time we move it so that it does not clash with the .d for path/foobar.i obtained from debug preprocessing
# of path/foobar.cpp
# path/foobar.i and path/foobar.h.i cannot be both built simultaneously, so the initial path/foobar.d obtained for both does
# not clash either
#
# $(D) is target-specific variable defined in dir_bottom.mk
# set to relative directory to the makefile fragment dir.mk that compiles this target
#
# CPPFLAGS are flags for the preprocessing of the target
################################################################################
define PREPROCESS=
$(QUIET)echo "preprocess $< -o $@"
$(QUIET)$(CXX) -E $($(D)/CXXFLAGS) $(filter-out -Werror, $(GCXXFLAGS)) $(CPPFLAGS) -MMD -MP -c  $< -o $@
$(QUIET)sed 's|\b\w*\.o\b|$@|' <$(@:i=d) >$@.d
endef

%.i: %.cpp $$(INCL_GENERATED_CONTENTS) $$(MKFS)
	$(QUIET)$(PREPROCESS)

%.i: %.cxx $$(INCL_GENERATED_CONTENTS) $$(MKFS)
	$(QUIET)$(PREPROCESS)

%.i: %.c $$(INCL_GENERATED_CONTENTS) $$(MKFS)
	$(QUIET)$(PREPROCESS)

.PRECIOUS: %.h.i
%.h.i: %.h $$(INCL_GENERATED_CONTENTS) $$(MKFS)
	$(QUIET)$(QUIET)$(PREPROCESS)

.PRECIOUS: SWIG/%_wrap.i


################################################################################
# include generated dependencies
#
# try and not read too many files
# if you only want objects or preprocessed files, then read only those dependencies
# in this directory
################################################################################

GEN_DEPS += $(OBJS:=.d)
GEN_DEPS := $(sort $(GEN_DEPS))
# Transform path/objetcs to their shortest path (no ../foo/../..)
GEN_DEPS := $(abspath $(GEN_DEPS))
GEN_DEPS := $(subst $(CURDIR)/,,$(GEN_DEPS))

-include $(if $(MAKECMDGOALS),$(if $(filter-out %.o %.i,$(MAKECMDGOALS))$(filter %.gen.i,$(MAKECMDGOALS)),$(GEN_DEPS), $(MAKECMDGOALS:=.d)),$(GEN_DEPS))

#we rely on that $(MAKECMDGOALS) does not have any preprocessed goals,
# which are just for debugging
MAKECMDGOALS_PREPROCESSED := $(filter-out %.gen.i, $(filter %.i, $(MAKECMDGOALS)))
ifneq '$(MAKECMDGOALS_PREPROCESSED)' ''
-include $(MAKECMDGOALS_PREPROCESSED:=.d)
endif

################################################################################
# $(call MAKE_DEPENDABLE,FOOBAR)
#
# (note there is no space in front of FOOBAR)
# makes variable FOOBAR "dependable", as if it were a file, so you can say
#
# target: FOOBAR
#		recipe
#
# and the target will be rebuilt if the value of FOOBAR changes
#
#
# in fact this is implemented with a file named FOOBAR, so we should not have such files (with all cap names)
# in this directory
################################################################################
define MAKE_DEPENDABLE
$(eval $(shell \
	 if [ ! -e $1 -o "$(head -n 1 $1)" != "$($1)" ]; \
		then echo -n $($1) > $1; \
	fi \
	) DEPENDABLE_VARS += $1)
endef

$(call MAKE_DEPENDABLE,DBG)
$(call MAKE_DEPENDABLE,SM)


################################################################################
# explicit rule to make filelist.$1.customer.txt for product $1
#
# if variable <product>_FILES=foobar.txt was specified on command line
# then to be a file in products/distFiles
# then filelist.<product>.customer.txt is copied
# from products/distFiles/foobar.txt
# so you can put any distribution lists you want in foobar.txt
#
# if that variable is not defined, then filelist.<product>.customer.txt is built as normal
#
# in the process we make the variable $1_FILES dependable
################################################################################
define DEFINE_FILELIST_PROD_CUSTOMER

$(eval $(call MAKE_DEPENDABLE,$1_FILES))

filelist.$1.customer.txt: $1_FILES

ifdef $1_FILES
filelist.$1.customer.txt: products/distFiles/$($1_FILES) Makefile
	cp $$< $$@
endif

endef

$(foreach prod,$(MOD_PRODUCTS),$(eval $(call DEFINE_FILELIST_PROD_CUSTOMER,$(prod))))
 A:= $(shell echo $(prod))
$(info $(A))

usage help:
	@$(ECHO)
	@$(ECHO) build products:
	@$(foreach goal, $(DEFAULT_PRODUCTS), echo 'xdk>maker $(goal)';)
	@$(ECHO)
	@$(ECHO) 'xdk>maker sdklibs xpSaiAdapter xpsApp xpSaiApp saiShell_py sdk_py'
	@$(ECHO)
	@$(ECHO) 'xdk>maker cpss'
	@$(ECHO) 'xdk>maker cpss_build_info'
	@$(ECHO) 'xdk>maker cpss_clean_build'
	@$(ECHO) 'xdk>maker cpss_apply_patch'
	@$(ECHO) 'xdk>maker cpss_build'
	@$(ECHO) 'xdk>maker cpss_build_force_unzip'
	@$(ECHO) 'xdk>maker cpss_clean'
	@$(ECHO) 'xdk>maker cpss_clean_all'
	@$(ECHO) 'xdk>maker cpss_clean_build'
	@$(ECHO) 'xdk>maker cpss_clean_libs'
	@$(ECHO) 'xdk>maker cpss_rebuild'
	@$(ECHO)
	@$(ECHO) 'xdk>maker clean'
	@$(ECHO) 'xdk>maker cleanall'
	@$(ECHO)
	@$(ECHO) 'xdk>maker buildall'
	@$(ECHO)
	@$(ECHO) 'cross compile for armada 385 Aldrin-2 32-bit'
	@$(ECHO) 'xdk>maker TARGET=a385 [target name]'
	@$(ECHO) 'cross compile for armada 38x Aldrin 64bit'
	@$(ECHO) 'xdk>maker TARGET=aarch64 [target name]'
	@$(ECHO)

buildall : saiPreproc  cpss sdklibs xpSaiAdapter xpsApp xpSaiApp saiShell_py sdk_py saiLuaWrapper
	@$(ECHO)
	@$(ECHO) '#####################################################'
	@$(ECHO) '#  	BUILT ALL TARGETS                             #'
	@$(ECHO) '#####################################################'
	@$(ECHO)

# Clean the build
# there is more to it, where "black-boxes" are built, they are cleaned there as well
cleanall : clean cpss_clean_all
	$(QUIET)$(RM) -rf $(DIST)/*
	$(QUIET)$(RM) -rf objs/*

clean : $(XP_CLEAN) cpss_clean
	@$(ECHO) Cleaning the build, removing all symbols, libraries, and binaries
	@$(ECHO)
	$(QUIET)$(RM) -rf $(OBJSDIR) cscope.*
	$(QUIET)$(RM) -rf $(DIST)

	$(QUIET)find ../ -name '*.h.i' -exec rm '{}' ';'
	$(QUIET)find ../ -name '*.d' -exec rm '{}' ';'
	$(QUIET)find ../ -name '*.pyc' -exec rm '{}' ';'
	$(QUIET)$(RM) -f ../cpss/.config
	$(QUIET)$(RM) -f SWIG/*_modified.*
	$(QUIET)$(RM) -f SWIG/*_wrap.c*
	$(QUIET)$(RM) -f SWIG/*_wrap.i*
	$(QUIET)$(RM) -f SWIG/*.gen.*
	$(QUIET)$(RM) -f SWIG/*.top.*
	$(QUIET)$(RM) -f SWIG/*.front.txt
	$(QUIET)$(RM) -f $(DEPENDABLE_VARS)
	$(QUIET)$(RM) -f *.o
	$(QUIET)$(RM) -f *_FILES
	$(QUIET)$(RM) -fr demo
	@$(ECHO)
	@$(ECHO) The clean has finished


# directory clean
.PHONY: %/clean
%/clean:
	@$(ECHO) Cleaning up $* ...
	-$(RM) -f $(OBJSDIR)/$*/*.a $(OBJSDIR)/$*/*.o
	-$(RM) -f $*/filelist.*
