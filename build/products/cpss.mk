########################################
# this $(PRODUCT).mk file has stuff specific to $(PRODUCT)
########################################

PRODUCT := $(FILENAME_BASE)

$(PRODUCT): cpss_build

CPSS_BUILD_SH=cpss/build_cpss.sh
CPSS_BUILD_FW=cpss/extension/srvCpu/firmware/internal/build_free_rtos.sh


# Default AARCH
CPSS_ARCH := i386_64
LIB_ARCH := i386_64
# ARCH==ARM
ifeq ($(TARGET),a385)
    CPSS_ARCH := ARMv7_LK_3_10
    LIB_ARCH := ARMV7
endif
ifeq ($(TARGET),aarch64)
    CPSS_ARCH := AARCH64v8
    LIB_ARCH := AARCH64v8
endif
ifeq ($(TARGET),x55aarch64)
    CPSS_ARCH := AARCH64v8
    LIB_ARCH := AARCH64v8
endif
ifeq ($(TARGET),x86_64)
    CPSS_ARCH := INTEL64
    LIB_ARCH := INTEL64
endif


#####################################################
#
##  CPSS OPTIONS - Configuration start
#
#####################################################

#CPSS_ENABLER_MODE := CPSS_ENABLER_NOKM CPSS_ENABLER_SHARED_NOKM # CPSS_LIB CPSS_LIB_SHARED
CPSS_MPD := MPD_NO
ifeq ($(TARGET),aarch64)
	CPSS_ENABLER_MODE := CPSS_ENABLER_NOKM
    CPSS_MPD :=
endif
ifeq ($(TARGET),a385)
	CPSS_ENABLER_MODE := CPSS_ENABLER_NOKM
endif
ifeq ($(TARGET),x86_64)
	CPSS_ENABLER_MODE := CPSS_ENABLER_NOKM
endif
CPSS_FLAGS := UTF_NO 
#CPSS_NOLUA := NOLUA
#CPSS_LOG := CPSS_LOG_YES
CPSS_DEBUG_ON:=D_OFF
#CPSS_DEBUG_ON:=D_ON
ifdef VERBOSE
    CPSS_COMPILE_LOG := VERBOSE
endif

CPSS_OPTIONS := DX_ALL $(CPSS_COMPILE_LOG) $(CPSS_ARCH) NOKERNEL $(CPSS_NOLUA) $(CPSS_ENABLER_MODE) $(CPSS_FLAGS) $(CPSS_LOG) EZ_BRINGUP_NO $(CPSS_MPD)

#####################################################
#
##  CPSS OPTIONS - Configuration End
#
#####################################################


CPSS_LDFLAG += -lrt -ldl -rdynamic
ARCSTFLAGS:= rcsT
COMPILATION_ROOT := $(SUBDIR)/$(OBJSDIR)/../cpss/
CPSS_LIB_PATH := $(COMPILATION_ROOT)
CPSS_LIBS := $(CPSS_LIB_PATH)/common/libs//COMMON.a
CPSS_LIBS += $(CPSS_LIB_PATH)/mainPpDrv/libs//MAINPPDRV.a
CPSS_LIBS += $(CPSS_LIB_PATH)/mainExtUtils/libs//MAINEXTUTILS.a
#CPSS_LIBS += $(CPSS_LIB_PATH)/embeddedCommands/libs//EMBEDDEDCOMMANDS.a
ifeq ($(CPSS_ARCH),i386_64)
CPSS_LIBS += $(CPSS_LIB_PATH)/simulation/libs/SIMULATION.a
endif
CPSS_LIBS += $(CPSS_LIB_PATH)/mainTmDrv/libs//MAINTMDRV.a
#CPSS_LIBS += $(CPSS_LIB_PATH)/mainOs/libs/$(LIB_ARCH)/mainOs.a
CPSS_LIBS += $(CPSS_LIB_PATH)/mainGaltisWrapper/libs//MAINGALTISWRAPPER.a
ifneq ($(CPSS_NOLUA),NOLUA)
CPSS_LIBS += $(CPSS_LIB_PATH)/mainLuaWrapper/libs//MAINLUAWRAPPER.a
endif

CPSS_ENABLER := $(CPSS_LIB_PATH)/cpssEnabler/libs//CPSSENABLER.a

CPSS_ENABLER_LIB:=  libCpssEnablerNoMain.a
CPSS_ENABLER_EXT_LIB:=  libCpssEnablerExtNoMain.a
CPSS_VERSION_LIB:=  libCpssVersion.a
XDK_CPSS_LIB:=  libXdkCpss.a
XDK_CPSS_SHARED_LIB := $(XDK_CPSS_LIB:.a=.so)

cpss_clean_patch: D:= $D
cpss_clean_patch:
ifneq ("$(wildcard $(SUBDIR)/cpss_patch_done)","")
	@$(ECHO)
	@$(ECHO) ^^^^^ TARGET: $@
	cd ../cpss && cat ../build/cpss_patches/tcam_warm_restart.patch | patch -p1 -R
	cd ../cpss && cat ../build/cpss_patches/lua_login_auth.diff | patch -p1 -R
	cd ../cpss && cat ../build/cpss_patches/cpss_maker.patch | patch -p1 -R
	cd ../cpss && cat ../build/cpss_patches/04_dma_verbose_prints.patch | patch -p1 -R
	cd ../cpss && cat ../build/cpss_patches/shared_mem.patch | patch -p1 -R
	cd ../cpss && cat ../build/cpss_patches/sem_Lock_WM_Fail.patch | patch -p1 -R
	cd ../cpss && cat ../build/cpss_patches/ac5x_pcl.patch | patch -p1 -R
	cd ../cpss && cat ../build/cpss_patches/AAC_memory_access_in_FDBMgr.patch| patch -p1 -R
	cd ../cpss && cat ../build/cpss_patches/CPSS-14308.patch | patch -p1 -R
	cd ../cpss && cat ../build/cpss_patches/CPSS-14472.patch | patch -p1 -R
	cd ../cpss && cat ../build/cpss_patches/CPSS-14467.patch | patch -p1 -R
	cd ../cpss && cat ../build/cpss_patches/CPSS-14153.patch | patch -p1 -R
	cd ../cpss && cat ../build/cpss_patches/HA_pciex.patch | patch -p1 -R
	cd ../cpss && cat ../build/cpss_patches/intr_pci_bus_master.patch | patch -p1 -R
	cd ../cpss && cat ../build/cpss_patches/CPSS-14615.patch | patch -p1 -R
	cd ../cpss && cat ../build/cpss_patches/CPSS-14537.patch | patch -p1 -R
	cd ../cpss && cat ../build/cpss_patches/CPSS-14256_1.patch | patch -p1 -R
	cd ../cpss && cat ../build/cpss_patches/CPSS-14256_2.patch | patch -p1 -R
	cd ../cpss && cat ../build/cpss_patches/CPSS-14664.patch | patch -p1 -R
	cd ../cpss && cat ../build/cpss_patches/CPSS-13588.patch | patch -p1 -R
	cd ../cpss && cat ../build/cpss_patches/CPSS-14628.patch | patch -p1 -R
	$(ECHO) 
ifneq ("$(wildcard ../$(CPSS_BUILD_FW))","")
	$(ECHO) "Cleaning excluded file firmware patches"
	cd ../cpss && cat ../build/cpss_patches/CPSS-14692.patch | patch -p1 -R
	$(ECHO)
endif
	rm $(SUBDIR)/cpss_patch_done
endif

$(SUBDIR)/cpss_patch_done: D:= $D
$(SUBDIR)/cpss_patch_done:
	@$(ECHO)
	@$(ECHO) ^^^^^ TARGET: $@
	@$(ECHO) "============================================================"
	cd ../cpss && cat ../build/cpss_patches/04_dma_verbose_prints.patch | patch -p1
	cd ../cpss && cat ../build/cpss_patches/cpss_maker.patch | patch -p1
	cd ../cpss && cat ../build/cpss_patches/lua_login_auth.diff | patch -p1
	cd ../cpss && cat ../build/cpss_patches/shared_mem.patch | patch -p1
	cd ../cpss && cat ../build/cpss_patches/sem_Lock_WM_Fail.patch | patch -p1
	cd ../cpss && cat ../build/cpss_patches/ac5x_pcl.patch | patch -p1
	cd ../cpss && cat ../build/cpss_patches/AAC_memory_access_in_FDBMgr.patch | patch -p1
	cd ../cpss && cat ../build/cpss_patches/CPSS-14308.patch | patch -p1
	cd ../cpss && cat ../build/cpss_patches/CPSS-14472.patch | patch -p1
	cd ../cpss && cat ../build/cpss_patches/CPSS-14467.patch | patch -p1
	cd ../cpss && cat ../build/cpss_patches/CPSS-14153.patch | patch -p1
	cd ../cpss && cat ../build/cpss_patches/tcam_warm_restart.patch | patch -p1
	cd ../cpss && cat ../build/cpss_patches/intr_pci_bus_master.patch | patch -p1
	cd ../cpss && cat ../build/cpss_patches/HA_pciex.patch | patch -p1
	cd ../cpss && cat ../build/cpss_patches/CPSS-14615.patch | patch -p1
	cd ../cpss && cat ../build/cpss_patches/CPSS-14537.patch | patch -p1
	cd ../cpss && cat ../build/cpss_patches/CPSS-14256_1.patch | patch -p1
	cd ../cpss && cat ../build/cpss_patches/CPSS-14256_2.patch | patch -p1
	cd ../cpss && cat ../build/cpss_patches/CPSS-14664.patch | patch -p1
	cd ../cpss && cat ../build/cpss_patches/CPSS-13588.patch | patch -p1
	cd ../cpss && cat ../build/cpss_patches/CPSS-14628.patch | patch -p1
	@$(ECHO) "============================================================"
ifneq ("$(wildcard ../$(CPSS_BUILD_FW))","")
	@$(ECHO) "Apply excluded file firmware patches"
	cd ../cpss && cat ../build/cpss_patches/CPSS-14692.patch | patch -p1
else
	@$(ECHO) "Skiping excluded file firmware patches"
endif
	touch $(SUBDIR)/cpss_patch_done
	@$(ECHO)

cpss_apply_patch: D:= $D
cpss_apply_patch: $(SUBDIR)/cpss_patch_done

${CPSS_LIBS}: D := $D
${CPSS_LIBS}: | cpss_apply_patch $$(COMPILATION_ROOT)/.
	@$(ECHO)
	@$(ECHO) ^^^^^ TARGET: $@
	@$(ECHO) "CPSS: $D/${CPSS_BUILD_SH} $(CPSS_OPTIONS)"
        ifneq ("$(wildcard ../$(CPSS_BUILD_FW))","")
		@$(ECHO) "CPSS: building firmware"
		../${CPSS_BUILD_FW}
        else
		@$(ECHO) "CPSS: skipping firmware build"
        endif
	@export COMPILATION_ROOT=$(COMPILATION_ROOT) && export DEBUG_INFO=${CPSS_DEBUG_ON} && ../${CPSS_BUILD_SH} $(CPSS_OPTIONS)
	@$(ECHO)


$D/CPSS_BUILD: D := $D
$D/CPSS_BUILD: $(CPSS_LIBS)
	@$(ECHO)
	@$(ECHO) ^^^^^ TARGET: $@

$(DIST)/$(CPSS_ENABLER_LIB): D := $D
$(DIST)/$(CPSS_ENABLER_LIB): $D/CPSS_BUILD | $$(@D)/.
	@$(ECHO)
	@$(ECHO) ^^^^^ TARGET: $@
	#$(QUIET)$(AR) $(ARFLAGS) $(DIST)/$(CPSS_ENABLER_LIB) $(shell find $(CPSS_LIB_PATH)/cpssEnabler/mainOs/ -type f -name '*.o' )
	@cp $(CPSS_ENABLER) $(DIST)/$(CPSS_ENABLER_LIB)
	@$(STRIP) -N main $(DIST)/$(CPSS_ENABLER_LIB)
	@$(ECHO)

$(DIST)/$(XDK_CPSS_LIB): D := $D
$(DIST)/$(XDK_CPSS_LIB): $(DIST)/$(CPSS_ENABLER_LIB) | $$(@D)/.
	@$(ECHO)
	@$(ECHO) ^^^^^ TARGET: $@
	#$(QUIET)$(AR) $(ARCSTFLAGS) $(DIST)/$(XDK_CPSS_LIB) $(DIST)/$(CPSS_ENABLER_LIB) $(DIST)/$(CPSS_ENABLER_EXT_LIB) $(CPSS_LIBS)
	$(QUIET)rm -f $(DIST)/$(XDK_CPSS_LIB)
	$(QUIET)$(AR) $(ARFLAGS) $(DIST)/$(CPSS_VERSION_LIB) $(COMPILATION_ROOT)/version_info.o
	$(QUIET)$(AR) $(ARCSTFLAGS)  $(DIST)/$(XDK_CPSS_LIB) $(DIST)/$(CPSS_VERSION_LIB) $(DIST)/$(CPSS_ENABLER_LIB) $(CPSS_LIBS)
	@$(ECHO)
	@$(ECHO) "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	@$(ECHO) "+                                                                              +"
	@$(ECHO) "+       Building CPSS Static lib from ... $(CPSS_SRC_ZIP)      +"
	@$(ECHO) "+                                                                              +"
	@$(ECHO) "+                       The target ($@) is ready!      +"
	@$(ECHO) "+                                                                              +"
	@$(ECHO) "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	@$(ECHO)

$(DIST)/$(XDK_CPSS_SHARED_LIB): D := $D
$(DIST)/$(XDK_CPSS_SHARED_LIB): $(DIST)/$(CPSS_ENABLER_LIB) | $$(@D)/.
	@$(ECHO)
	@$(ECHO) ^^^^^ TARGET: $@
ifneq '$(SHARED)' ''
	#$(QUIET)$(CC) -shared -o $(DIST)/$(XDK_CPSS_SHARED_LIB) -Xlinker --whole-archive $(DIST)/$(CPSS_ENABLER_LIB) $(DIST)/$(CPSS_ENABLER_EXT_LIB) $(CPSS_LIBS) -Xlinker --no-whole-archive
	$(QUIET)$(CC) -fPIC -rdynamic $(DIST)/$(CPSS_ENABLER_LIB) $(CPSS_LIBS) $(DIST)/$(CPSS_VERSION_LIB)  -shared -o $(DIST)/$(XDK_CPSS_SHARED_LIB)
	@$(ECHO)
	@$(ECHO) "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	@$(ECHO) "+                                                                              +"
	@$(ECHO) "+     Building CPSS Shared lib from ... $(CPSS_SRC_ZIP)        +"
	@$(ECHO) "+                                                                              +"
	@$(ECHO) "+                       The target ($@) is ready!    +"
	@$(ECHO) "+                                                                              +"
	@$(ECHO) "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
endif
	@$(ECHO)

cpss_build_info: D := $D
cpss_build_info:
	@$(ECHO)
	@$(ECHO) ^^^^^ TARGET: $@
	@$(ECHO) "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	@$(ECHO) "+                                                                              +"
	@$(ECHO)   ++  CROSS_COMPILE=${CROSS_COMPILE}
	@$(ECHO)   ++  SYSROOT=${SYSROOT}
	@$(ECHO)   ++  CFLAGS=${CFLAGS}
	@$(ECHO)   ++  PATH=${PATH}
	@$(ECHO)   ++  CPSS_ARCH=${CPSS_ARCH}
	@$(ECHO)   ++  CPSS_OPTIONS=${CPSS_OPTIONS}
	@$(ECHO)   "export COMPILATION_ROOT=$(COMPILATION_ROOT) && export DEBUG_INFO=${CPSS_DEBUG_ON} && ../${CPSS_BUILD_SH} $(CPSS_OPTIONS)"
	@$(ECHO) "+                                                                              +"
	@$(ECHO) "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	@$(ECHO)


cpss_build: D := $D
cpss_build: cpss_build_info $(DIST)/$(XDK_CPSS_LIB) $(DIST)/$(XDK_CPSS_SHARED_LIB)
	@$(ECHO)
	@$(ECHO) ^^^^^ TARGET: $@
	@$(ECHO) "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	@$(ECHO) "+                                                                              +"
	@$(ECHO) "+            Building CPSS ... $D/${CPSS_SRC_ZIP}        +"
	@$(ECHO) "+                                                                              +"
	@$(ECHO) "+                       The target ($@) is ready!                      +"
	@$(ECHO) "+                                                                              +"
	@$(ECHO) "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	@$(ECHO)

.PHONY: cpss_clean_build cpss_clean

cpss_clean_libs: D := $D
cpss_clean_libs :
	@$(ECHO)
	@$(ECHO) ^^^^^ TARGET: $@
	@$(ECHO) Cleaning up libs $@ ...
	$(QUIET)$(RM) -f ${CPSS_LIBS} $(DIST)/$(CPSS_ENABLER_LIB) $(DIST)/$(CPSS_ENABLER_EXT_LIB)
	@$(ECHO)

cpss_rebuild: D := $D
cpss_rebuild: | cpss_clean_libs ${CPSS_LIBS}  $(DIST)/$(XDK_CPSS_LIB) $(DIST)/$(XDK_CPSS_SHARED_LIB)
	@$(ECHO)
	@$(ECHO) ^^^^^ TARGET: $@
	@$(ECHO)

cpss_clean_build: D := $D
cpss_clean_build: | cpss_clean ${CPSS_LIBS}
	@$(ECHO)
	@$(ECHO) ^^^^^ TARGET: $@
	@$(ECHO)

cpss_clean: D := $D
cpss_clean : cpss_clean_patch
	@$(ECHO)
	@$(ECHO) Cleaning up $@ ...
	$(QUIET)$(RM) -f $(DIST)/$(XDK_CPSS_LIB)
	$(QUIET)$(RM) -f $(DIST)/$(XDK_CPSS_SHARED_LIB)
	$(QUIET)$(RM) -f $(DIST)/$(CPSS_ENABLER_LIB)
	$(QUIET)$(RM) -f ${CPSS_LIBS}
	$(QUIET)$(RM) -fr $(CPSS_LIB_PATH)
	@$(ECHO)

cpss_clean_all: D := $D
cpss_clean_all : cpss_clean
	@$(ECHO)

.PHONY: $D/clean
$(DIST)/clean: D := $D
$(DIST)/clean :
	@$(ECHO)
	@$(ECHO) ^^^^^ TARGET: $@
	@$(ECHO) Cleaning up $D ...
	$(QUIET)$(RM) -fr $(CPSS_LIB_PATH)
	@$(ECHO)


################################################################################
# put this immediately above bottom magic
# includes dir.mk files in these subdirectories
###############################################################################
$D/SUBDIRS := \

$(call INCL_SUBDIRS, $($D/SUBDIRS))

