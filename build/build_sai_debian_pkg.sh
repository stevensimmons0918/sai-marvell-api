#!/bin/bash
#arg 1 - sai version
#arg 2 - arch

version=$1
ARCH=$2
message=$3
DIST_ARCH="native"

DIST_DIR=../dist/
OBJS_DIR=./objs/
SAI_DBG_LIB_SHORT="mrvllibsaidbg"
SAI_LIB_SHORT="mrvllibsai"

GIT_XPS_COMMIT=0
GIT_XPS_COMMIT_MSG=""
GIT_SAI_COMMIT=0
GIT_CPSS_COMMIT=0

get_git_commit()
{
    pushd ../
    GIT_XPS_COMMIT=`git rev-parse --short HEAD`
    GIT_XPS_COMMIT_MSG=`git show --summary  --pretty=oneline --abbrev-commit`
    popd

    pushd ../plugins/sai/SAI
    GIT_SAI_COMMIT=`git rev-parse --short HEAD`
    popd

    pushd ../cpss/
    GIT_CPSS_COMMIT=`git rev-parse --short HEAD`
    popd
}

get_git_commit_from_file()
{
    input=git_info.txt
    index=0
    while IFS= read -r line
    do
      git_info[index]="$line"
      ((index=index+1))
    done < "$input"

    GIT_XPS_COMMIT=${git_info[0]}
    GIT_XPS_COMMIT_MSG=${git_info[1]}
    GIT_SAI_COMMIT=${git_info[2]}
    GIT_CPSS_COMMIT=${git_info[3]}
}

build_deb_pkg()
{
    PKG_NAME=$1
    LIB_NAME=$2
    LIB_SO=$3
    LIB_SO_1=$4

    if [ ! -f ${DIST_DIR}/${LIB_SO} ]; then
        echo "${LIB_SO} Not found"
        exit -1
    fi


    GPIO_PATH='linux_5.10';

    if [[ $version =~ "1.7.1" ]]; then
           GPIO_PATH='linux_4.19'
           fi

    DATE=`date -u`
    cd $OBJS_DIR
   if [ ! -d "$LIB_NAME/usr/lib" ]; then
        mkdir -p $LIB_NAME/usr/lib
    fi
    cp ../${DIST_DIR}/${DIST_ARCH}/${LIB_SO} $LIB_NAME/usr/lib/
    if [ -n "$LIB_SO_1" ]
    then
        cp ../${DIST_DIR}/${DIST_ARCH}/${LIB_SO_1} $LIB_NAME/usr/lib/
    fi

    if [ ! -d "$LIB_NAME/DEBIAN" ]; then
        mkdir -p $LIB_NAME/DEBIAN
    fi
    echo "Package: $PKG_NAME " > $LIB_NAME/DEBIAN/control
    echo "Version: $version" >> $LIB_NAME/DEBIAN/control
    echo "Section: base" >> $LIB_NAME/DEBIAN/control
    echo "Priority: optional" >> $LIB_NAME/DEBIAN/control
    echo "Architecture: $ARCH" >> $LIB_NAME/DEBIAN/control
    if [ $PKG_NAME = $SAI_LIB_SHORT ]; then
        if [ ! -d "$LIB_NAME/usr/include/sai" ]; then
            mkdir -p ${LIB_NAME}/usr/include/sai/
        fi
        cp ../../plugins/sai/SAI/xpSai/sai/inc/*.h $LIB_NAME/usr/include/sai/
        cp ../../plugins/sai/SAI/xpSai/sai/experimental/*.h $LIB_NAME/usr/include/sai/
        #cp ../../plugins/sai/SAI/xpSai/sai/test/saithrift/src/switch_sai_rpc_server.h $LIB_NAME/usr/include/

        if [ "${ARCH}" == "armhf" ]; then
            if [ ! -d "$LIB_NAME/lib/modules" ]; then
                mkdir -p $LIB_NAME/lib/modules/
            fi
            cp -f ../../kernel/gpio/$GPIO_PATH/mvGpioDrv.ko $LIB_NAME/lib/modules/
        fi
 
        if [ "${ARCH}" == "amd64" ]; then
            if [ ! -d "$LIB_NAME/iniFiles" ]; then
                mkdir -p ${LIB_NAME}/iniFiles
            fi
            if [ ! -d "$LIB_NAME//cpss/simulation/registerFiles" ]; then
                mkdir -p ${LIB_NAME}//cpss/simulation/registerFiles
                cp ../../iniFiles/* $LIB_NAME/iniFiles/
                cp -r ../../cpss/simulation/registerFiles/* $LIB_NAME/cpss/simulation/registerFiles/
            fi
       fi

       if [ "${ARCH}" == "arm64" ]; then
            if [ ! -d "$LIB_NAME/fwFiles/phy" ]; then
                mkdir -p ${LIB_NAME}/fwFiles/phy
                cp ../../fwFiles/phy/* $LIB_NAME/fwFiles/phy/
            fi
       fi
    else
        echo "Depends:" >> $LIB_NAME/DEBIAN/control
    fi
    echo "Maintainer: $USER <$USER@marvell.com>" >> $LIB_NAME/DEBIAN/control
    echo "Date: $DATE" >> $LIB_NAME/DEBIAN/control 
    echo "Description: XPSAI for marvell asic based on sai v$version" >> $LIB_NAME/DEBIAN/control
    echo "           XPS commit $GIT_XPS_COMMIT" >> $LIB_NAME/DEBIAN/control
    echo "           SAI commit $GIT_SAI_COMMIT" >> $LIB_NAME/DEBIAN/control
    echo "           CPSS commit $GIT_CPSS_COMMIT" >> $LIB_NAME/DEBIAN/control
    echo "           $message" >> $LIB_NAME/DEBIAN/control

    cat $LIB_NAME/DEBIAN/control
    echo "Building deb package"
    dpkg-deb --build $LIB_NAME/

    cd - 

    cp -f ${OBJS_DIR}/${LIB_NAME}.deb ${DIST_DIR}/
}

printpkginfo()
{
    echo 
    echo "========================================================"
    echo "Verbose Info"
    echo version=$version
    echo ARCH=$ARCH
    echo DIST_ARCH=$DIST_ARCH
    echo message=$message
    echo LAST_BUILT_ARCH[Ignored]=$TARGET
    echo "========================================================"
    echo 
    echo 
    echo "========================================================"
    echo "Package Info"
    echo "++ version=${version}"
    echo "++ ARCH=${ARCH}  "
    echo "++ Description=${message}  "
    echo "XPSAI source "
    echo "++ XPS commit $GIT_XPS_COMMIT"
    echo "++ SAI commit $GIT_SAI_COMMIT"
    echo "++ CPSS  commit $GIT_CPSS_COMMIT"
    echo "========================================================"
    echo 
}

getbuildtarget()
{
    TARGET=`ls ${DIST_DIR}/LASTTARGET-* | sed 's/.*-//'`
    if [ -z "$TARGET" ]; then
        echo "Error: invalid ARCH=$ARCH TARGET=${TARGET}"
        exit 1
    elif [ "$TARGET" = "native" ]; then
        ARCH="amd64"
    elif [ "$TARGET" = "a385" ]; then
        ARCH="armhf"
    elif [ "$TARGET" = "aarch64" ]; then
        ARCH="arm64"
    elif [ "$TARGET" = "x55aarch64" ]; then
        ARCH="x55aarch64"
    else
        ARCH="$TARGET"
    fi
}

getdistarch()
{
    TARGET=`ls ${DIST_DIR}/LASTTARGET-* | sed 's/.*-//'`
    if [ "$ARCH" = "amd64" ]; then
        DIST_ARCH="x86_64"
        if [ "$TARGET" = "native" ]; then
            DIST_ARCH="native"
        fi
    elif [ "$ARCH" = "armhf" ]; then
        DIST_ARCH="a385"
    elif [ "$ARCH" = "arm64" ]; then
        DIST_ARCH="aarch64"
    elif [ "$ARCH" = "x55aarch64" ]; then
        DIST_ARCH="x55aarch64"
    else
        DIST_ARCH="$TARGET"
    fi
}

main()
{

    if [ $# -eq 0 ]
    then
        echo "Usage: $0 <version> <arch> <description>"
        exit 1
    fi

    FILE=git_info.txt
    if test -f "$FILE"; then
        get_git_commit_from_file
    else
        get_git_commit
    fi

    if [ -z "$version" ]
    then
        version="1.4.99"
    fi
    if [ -z "$ARCH" ]
    then
        getbuildtarget
    fi
    if [ -z "$message" ]
    then
        message="${GIT_XPS_COMMIT_MSG}"
    fi
    getdistarch
    if [ $# -lt 3 ]
    then
        echo -e "invalid arguments\n usage: build_sai_debian_pkg.sh <version> <architecture> <package_information>"
        echo -e "   <package_information> describes about package"
        echo -e "   For multiple line use \"\\\n\". And each newline starts with whitespace.\n    Ex: \"  Package fixes\\\n   1. bug1\\\n  2. bug2\""
        echo 
        echo "Using defaults"
        #exit 0
    fi
    printpkginfo
    echo

    SAI_LIB_NAME="mrvllibsai_${version}_${ARCH}"
    SAI_LIB_SO="libsai.so"
    FPA_LIB_SO="libXdkCpss.so"

    SAI_DBG_LIB_NAME="mrvllibsai_dbg_${version}_${ARCH}"
    SAI_DBG_LIB_SO="libsai.so.dbg"

    # sai.so
    build_deb_pkg $SAI_LIB_SHORT $SAI_LIB_NAME $SAI_LIB_SO $FPA_LIB_SO

    # sai.so.dbg
    build_deb_pkg $SAI_DBG_LIB_SHORT $SAI_DBG_LIB_NAME $SAI_DBG_LIB_SO

    echo
    echo "Done."
    echo
}


main $@
