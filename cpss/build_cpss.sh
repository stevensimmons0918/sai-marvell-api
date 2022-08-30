#!/usr/bin/env bash
##################################################################################
##  build_cpss.sh
##################################################################################
##
##  APPDEMO IMAGE BUILD WRAPPER
##
##################################################################################
## ARGUMENTS         :  Can be defined in any order
##
##     CPU and  PP_TYPE  are the mandatory parameters  -  the rest are optional
##
##      MANDATORY PARAMETERS
##      -----------------------------------
## CPU      : i386 i386_64 MIPS64 INTEL64 INTEL64_32 INTEL32 ARMv7_LK_4_4 ARMv7_LK_3_10 AARCH64v8
##
## PP_TYPE  : DX_ALL PX_ALL DXPX
##
##      OPTIONAL PARAMETERS
##      -------------------------------
##
## PRODUCT           :  CPSS_ENABLER  CPSS_LIB  CPSS_ENABLER_NOKM CPSS_BLOB CPSS_USE_MUTEX_PROFILER
##                      CPSS_ENABLER_SHARED CPSS_LIB_SHARED CPSS_ENABLER_SHARED_NOKM
##                      CPSS_APP_REF CPSS_APP_LIB
##                      - default value CPSS_ENABLER
##
## UT                :  UTF_YES UTF_NO       - default value UTF_YES  ( build UT )
## TM                :  TM_YES TM_NO         - default value TM_YES   ( build TM )
## CPSS_LOG          :  CPSS_LOG_YES CPSS_LOG_NO - default value  CPSS_LOG_YES
##
## EZ_BRINGUP        :  EZ_BRINGUP_YES by default EZ_BRINGUP code is included, EZ_BRINGUP_NO - no EZ_BRINGUP compilation/support
##
## MPD               :  MPD_YES by default MPD code is included, MPD_NO - no MPD compilation/support
##
## LUA               :  NOLUA (the same as LUA_NO) -  by default LUA code is included, NOLUA removes LUA support
##
## NO_EXAMPLES       :  remove examples from LUA code
##
## GALTIS            :  NOGALTIS - galtis code not used
##
## TOOLKIT           :  SDK_5_V5 SDK_5_V7 SDK_3.2 SDK_1.2.4 ELDK_3.1 YOCTO
##
## Build options     :  UNZIP   ENVBASE  CUST  SHLIB SO_LIB     - no defaulf value
##
## Build options     :  UNZIP   ENVBASE  CUST  SHLIB      - no defaulf value
##
## GM_LION           :  specify using of   GM libs for DX LION simulation image build
##
## GM_BOBCAT2        :  specify using of   GM libs for GM_BOBCAT2 simulation image build
##
## GM_BOBCAT2_B0     :  specify using of   GM libs for GM_BOBCAT2_B0 simulation image build
##
## GM_BOBCAT3        :  specify using of   GM libs for GM_BOBCAT3 simulation image build
##
## GM_ALDRIN2        :  specify using of   GM libs for GM_ALDRIN2 simulation image build
##
## GM_FALCON         :  specify using of   GM libs for GM_FALCON simulation image build
##
## asim/ASIM         :  specify using of ASIM moded of compilation :Hybrid of 'ARM' and 'ASIC_SIMULATION'
##
## NOKERNEL            :  Do not build kernel
##
## NOLINK              :  Only build libs - do not try to link
##
## LEGACY|legacy     :  legacy "slow" compilation options (no defaulf value)
##
## Build CPSS log compilation type
##                   :   silent|SILENT  - not output of compilation log
##                   :   info|INFO   - output c-files only in compilation log (as default)
##                   :   verbose|VERBOSE - full output compilation log
##
##  DEPENDENCY       : DEP_YES - image will be compiled with dependency files (as default)
##                   : DEP_NO  - without dependency files
##
##
## HIR               : Build App for Hot Insertion/Removal
##################################################
##
##  ENVIRONMENT VARIABLES
##
##  The following parameters can be define as environment variables:
##
## --------------------------------------------------------------------------------------
## variable                         default value
## --------------------------------------------------------------------------------------
##  WORKING_DIRECTORY             currect directory (pwd)
##  CPSS_PATH                     /cpss/SW/prestera
##  LINUX_DIST_PATH               $WORKING_DIRECTORY/linux
##  LSP_USER_BASE                 $WORKING_DIRECTORY/lsp
##  COMPILATION_ROOT              $WORKING_DIRECTORY/compilation_root
##  IMAGE_PATH                    $WORKING_DIRECTORY/compilation_root
##  LINUX_BUILD_KERNEL            ""
##  DONT_LINK                     ""
##  TARGET_DIR_ROOTFS             ""
## -----------------------------------------------------------------------------------------
##
##       OPTIONS:
##
##               UNZIP  - this option can be used by customer to:
##                                 - unzip CPSS components
##                                 - compile CPSS modules
##                                 - build CPSS appDemo
##
##
##               ENVBASE  - this option can be used by customer to set CPSS build environment
##
##
##      EXAMPLES:
##
##-----------------------------------------------------------------------------------------------
##          ${CPSS_PATH}/build_cpss.sh   DX_ALL   PPC603
##
##              appDemo  goes to ./compilation_root , objects   in  ./compilation_root
##
##------------------------------------------------------------------------------------------------
##
##          ${CPSS_PATH}/build_cpss.sh   DX_ALL   PPC85XX UNZIP
##
##          The following  zip files should be located in current working directory:
##              - CPSS-PP-DxCh***.zip
##              - CPSS-FA-Xbar***.zip
##              - CPSS-FX***.zip
##              - EnablerSuite***.zip
##              - ExtUtils***.zip
##              - GaltisSuite***.zip
##              - ReferenceCode***.zip
##              - UT*.zip
##              - lsp*.zip
##
##              The script :
##                  - unpack CPSS code to directory     ./unpack/cpss
##                  - unpack lsp to directory ./lsp
##                  - defines
##                              export  CPSS_PATH= ./unpack/cpss
##                              export  LSP_USER_BASE= ./unpack/lsp
##                  - build   DX_ALL   appDemo   in  ./compilation_root
##
##------------------------------------------------------------------------------------------------
##
##
##          ${CPSS_PATH}/build_cpss.sh   DX_ALL   PPC85XX   ENVBASE
##             The script set CPSS build environment for defined CPU and PP_TYPE and
##             gives the prompt. In thia case a customer can work in the CPSS build
##             environment
##
##------------------------------------------------------------------------------------------------
##
##          ${CPSS_PATH}/build_cpss.sh   DX_ALL  XCAT SHLIB
##              Build CPSS Shared Lib appDemo in developer mode
##
##          ${CPSS_PATH}/build_cpss.sh   DX_ALL  XCAT SHLIB UNZIP
##              Build CPSS Shared Lib appDemo in client mode
##
##------------------------------------------------------------------------------------------------
##      No linux kernel build:
##             export LINUX_BUILD_KERNEL=NO
##      No appDemo link:
##             export DONT_LINK=NO
##      Directory to copy uImage and rootfs.tgz:
##             export TARGET_DIR_ROOTFS=path_Directory_to_copy
##
##          $Revision: 51 $
##
##------------------------------------------------------------------------------------------------
##
##  C_SINGLE_COMPILATION -  C file path for single compilation (no defaulf value)
##                   for example:
##                       export C_SINGLE_COMPILATION=common/src/cpss/driver/interrupts/cpssDrvComIntEvReqQueues.c
##
##  OBJ_FILE_PATH -  Object file path for it's single compilation (no defaulf value)
##                   for example:
##                       export OBJ_FILE_PATH=common/src/cpss/driver/interrupts/cpssDrvComIntEvReqQueues.o
##
##################################################################



#--------------------------------------------------

function DurationTime ()
{
    STARTTIME=$1
    TN=$2

    d=$( date +%T )
    TN1="$(date +%s%N)"
    ENDTIME="$d"
    # Time interval in nanoseconds
    T="$(($(date +%s%N)-TN))"

    # Seconds
    S="$((T/1000000000))"
    # Milliseconds
    M="$((T/1000000))"
    Min="$((S/60%60))"
    echo          ---$3------------
    echo   "Start    : $STARTTIME"
    echo   "Finish   : $ENDTIME"
    printf "Duration : %02d:%02d:%02d,%02d\n" "$((S/3600%24))" "$((S/60%60))" "$((S%60))" "$(($M - $S*1000))"
    echo          ---------------
}

# Support calling make from external folder, save caller and current path
PWD=`pwd`
if [ ${0%/*} != "." ]; then
    if [ $PWD != ${0%/*} ]; then
        CALLER_PATH="CALLER_PATH=$PWD"
        CPSS_DIR="-C ${0%/*}"
    fi
fi
#--------------------------------------------------
TN="$(date +%s%N)"
d=$( date +%T )
STARTTIME="$d"
echo "Start    : $STARTTIME $0"
GHIR_TYPE=0
#--------------------------------------------------


####################
# HELP option
####################
if [ "$1" = "help" ]; then
   echo ""
   echo "   COMMAND LINE FORMAT:"

   echo "      build_cpss.sh < Packet Processor > < CPU/Compiler > < Options >"
   echo ""
   echo "   CPU/Compiler:"
   echo "      i386 i386_64 MIPS64 INTEL64 INTEL64_32 INTEL32 ARMv7_LK_4_4 ARMv7_LK_3_10 AARCH64v8 AARCH64v8be "
   echo ""
   echo "   Packet Processor:"
   echo "      DX_ALL"
   echo ""
   echo "   OPTIONS:"
   echo "      Toolkit options:"
   echo "         eldk gnueabi gnueabi_be gnu eldk_3.1 sdk_1.2.4 sdk_3.2 sdk_5_v5 sdk_5_v7 "
   echo ""
   echo "      UT options:"
   echo "         UT_NO         - UT code not included "
   echo ""
   echo "      TM options:"
   echo "         TM_NO         - TM code not included "
   echo ""
   echo "      LUA options:"
   echo "         NOLUA         - Lua code not included "
   echo ""
   echo "         NO_EXAMPLES   - remove examples from LUA code "
   echo ""
   echo "      EZ_BRINGUP options:"
   echo "         EZ_BRINGUP_YES by default EZ_BRINGUP code is included, EZ_BRINGUP_NO - no EZ_BRINGUP compilation/support "
   echo "         NOTE: currently EZ_BRINGUP flags ignored for 'shared library' and it is not part of the compilation"
   echo ""
   echo "      MPD options:"
   echo "         MPD_YES by default MPD code is included, MPD_NO - no MPD compilation/support"
   echo ""
   echo "      Galtis options:"
   echo "         NOGALTIS      - Galtis code not included "
   echo ""
   echo "      GM options:"
   echo "         GM_LION GM_BOBCAT2 GM_BOBCAT2_B0 GM_BOBK_CAELUM GM_BOBCAT3 GM_ALDRIN2 GM_FALCON"
   echo ""
   echo "      Shared lib options:"
   echo "         shlib "
   echo ""
   echo "      Target product options:"
   echo "         CPSS_LIB CPSS_ENABLER CPSS_ENABLER_NOKM CPSS_BLOB CPSS_CHECK_CPP FREERTOS"
   echo ""
   echo "      SCRIPT MODE options:"
   echo "         UNZIP      - specifies the following steps: "
   echo "                       - unzip CPSS zip files to working directory "
   echo "                       - compile unziped source files "
   echo "                       - build CPSS appDemo "
   echo "         CUST       - specifies the following steps: "
   echo "                       - compile unziped source files "
   echo "                       - build CPSS appDemo "
   echo "     HIR options:"
   echo "         HIR     - specifies that Hot Insertion/Removal appDemo has to be built"
   echo ""
   echo "      Legacy slow compilation options:"
   echo "         LEGACY|legacy "
   echo "      Clean object options:"
   echo "         CLEAN|clean "

    exit 0
fi


dbg=""
utf="INCLUDE_UTF=y"
gal="INCLUDE_GALTIS=y"
lua=""
tm="INCLUDE_TM=y"
mtx=""
lck=""
log=""
shr=""
gm=""
asim=""
silent="-s"
target="appDemo"
TARGET=""
BUILD_FOLDER=""
ezb="INCLUDE_EZ_BRINGUP=y"
ext=""
## the MPD compilation need to be check on HW before opening this mode as default.
## because the HW compilation uses the PHY driver of '88x3240'
mpd="INCLUDE_MPD=y"

ask_lib="n"
ask_build="n"

for arg in "$@"
do
    case $arg in
        i386)
            TARGET="sim32"
            if [ "${mpd}" = "INCLUDE_MPD=n" ]; then
                mpd="INCLUDE_MPD=y"
            fi
            ;;
        i386_64)
            TARGET="sim64"
            if [ "${mpd}" = "INCLUDE_MPD=n" ]; then
                mpd="INCLUDE_MPD=y"
            fi
            ;;
        MIPS64)
            #TODO
            TARGET="mips"
            ;;
        INTEL64)
            TARGET="x86_64"
            ;;
        INTEL64_32)
            #TODO
            TARGET="x86_32"
            ;;
        INTEL32)
            #TODO
            TARGET="sim32"
            ;;
        ARMv7_LK_3_10 | ARMv7_LK_4_4)
            TARGET="armv7"
            ;;
        ARMv7be_LK_3_10 | ARMv7ֹbe_LK_4_4 | MSYS_BE)
            TARGET="armv7be"
            ;;
        AARCH64v8)
            TARGET="armv8"
            ;;
        AARCH64v8be)
            TARGET="armv8be"
            ;;
        ARMv7_BEYE_LK_3_10)
            TARGET="armv7_beye"
            ;;
        AARCH64v8_BEYE)
            TARGET="armv8_beye"
            ;;
        GCC10)
            TARGET="armv8_gcc10"
            ;;
        DX_ALL)
            FAMILY="DX"
            ;;
        PX_ALL)
            FAMILY="PX"
            tm=""
                        ezb=""
                        mpd=""
            ;;
        DXPX )
            FAMILY="DXPX"
            ezb=""
            mpd=""
            ;;
        UTF_NO)
            utf=""
            ;;
        UTF_YES)
            ;;
        TM_NO)
            tm=""
            ;;
        NOLUA | LUA_NO)
            lua="CMD_LUA_CLI=n"
            ;;
        CPSS_LOG_NO)
            log="LOG_ENABLE=n"
            ;;
        EZ_BRINGUP_NO)
            ezb=""
                        ;;
        MPD_NO)
                        mpd=""
            ;;
        NOGALTIS)
            gal=""
            ;;
        CPSS_ENABLER_SHARED | CPSS_LIB_SHARED | CPSS_ENABLER_SHARED_NOKM)
            shr="SHARED_MEMORY=y"
            ezb=""
                        mpd=""
            ;;
        CPSS_ENABLER*)
            ;;

        CPSS_TRAFFIC_API_LOCK_DISABLE)
            lck="TRAFFIC_API_LOCK_DISABLE=y"
            ;;
        CPSS_USE_MUTEX_PROFILER)
            mtx="USE_MUTEX_PROFILER=y"
            ;;

        CPSS_LIB | CPSS_BLOB )
            #TODO
            ;;

        CPSS_APP_REF )
            ask_build="y"
            ;;

        CPSS_APP_LIB )
            gal=""
            ask_build="y"
            utf=""
            ask_lib="y"
            ;;

        CLEAN | clean)
            target=clean
            ;;

        GM_BOBCAT3)
            gm="GM=bobcat3"
            ;;
        GM_ALDRIN2)
            gm="GM=aldrin2"
            ;;
        GM_HAWK)
            gm="GM=hawk"
            ;;
        GM_FALCON)
            gm="GM=falcon"
            ;;
        GM_IRONMAN)
            gm="GM=ironman"
            ;;
        NOLINK)
            #TODO
            ;;
        verbose | VERBOSE )
            silent=""
            ;;

        HIR | hir)
            #TODO
            hir_type="YES"
            GHIR_TYPE=1
            ;;

        NOKERNEL)
            ;;

        ASIM | asim)
            #hybrid of ARM and ASIC_SIMULATION , for running on Octeon ASIM environment
            asim="Y"
            ;;
        C005)
            ext="EXT_MODULE=C005"
            ;;
        *)
            echo "Unknown parameter ${arg}"
            exit 1
            ;;

   esac
done

if [ "${asim}" = "Y" ]; then
    asim="ASIC_SIMULATION=EXISTS ASIC_SIMULATION_ENV_FORBIDDEN=EXISTS"
fi


if [ "${DEBUG_INFO}" = "D_ON" ]; then
    dbg="DEBUG_INFO=y"
fi

if [ -z ${NUM_CORES} ]; then
    NUM_CORES=$(grep -c ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu)
fi

if [ ! -z ${COMPILATION_ROOT} ]; then
    BUILD_FOLDER="BUILD_FOLDER=${COMPILATION_ROOT}"
fi

if [ "${TARGET/*_/}" = "beye" ]; then
    echo "Run /local/store/BullseyeCoverage/bin/cov01 -1"
    "/local/store/BullseyeCoverage/bin/cov01 -1"
    export COVFILE="${COMPILATION_ROOT}/Bulls.cov"
    echo "COVFILE: ${COVFILE}"
fi

echo "make $CPSS_DIR TARGET=$TARGET FAMILY=$FAMILY ASK=${ask_build} ASK_LIB_BUILD=${ask_lib} $dbg $utf $gal $lua $ezb $mpd $tm $mtx $lck $log $shr $gm $asim $target $silent $ext -j ${NUM_CORES:-1} $CALLER_PATH $BUILD_FOLDER"
make $CPSS_DIR TARGET=$TARGET FAMILY=$FAMILY ASK=${ask_build} ASK_LIB_BUILD=${ask_lib} $dbg $utf $gal $lua $ezb $mpd $tm $mtx $log $shr $gm $asim $target $silent $ext -j ${NUM_CORES:-1} $CALLER_PATH $BUILD_FOLDER

if [ "${ext}" = "EXT_MODULE=C005" ]; then
    if [ ! "${BUILD_FOLDER}" ]; then
        currentBranch=$(cd $(dirname $0) && git symbolic-ref --short HEAD)
        currentExePath="${PWD}/compilation_root/${currentBranch}/${TARGET}_${FAMILY}"
    else
        currentExePath=$(cut -d "=" -f2 <<< $BUILD_FOLDER)
    fi
    echo "Updating appDemo name for C005 image"
    echo "mv ${currentExePath}/appDemo ${currentExePath}/appDemoC005"
    mv ${currentExePath}/appDemo ${currentExePath}/appDemoC005
fi

DurationTime  ${STARTTIME} ${TN}  $0
