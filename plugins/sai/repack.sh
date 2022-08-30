#!/bin/bash

#### The simlpe script that remove SAI code that goes with
#### released xdk and push custom one

set -e
set -u

# By default get any from current folder
SAI_TARBALL=$(ls | grep sai-rel-*tar* || true)
XDK_TARBALL=$(ls | grep xdk-rel-*tar* || true)
RELPKG_DIR=.
SAI_LINK="0"

show_usage() {
    echo "repack.sh --xdkrel=<XDKREL> --sairel=<SAIREL> [--link] [--destdir=<DESTDIR>]"
    echo "  --xdkrel=<XDKREL>   the release xdk package in tar.bz2 format"
    echo "  --sairel=<SAIREL>   the release sai package in tar.bz2 format"
    echo "  --destdir=<DESTDIR> optional, the directory where xdk will be ready with custom sai"
    echo "  --link              optional, link SAI instead of copy"
}

parse_argument () {
    for i in "$@"
    do
        case $i in
            --xdkrel*)
                XDK_TARBALL="${i#*=}"
                shift
            ;;
            --sairel*)
                SAI_TARBALL="${i#*=}"
                shift
            ;;
            --destdir*)
                RELPKG_DIR="${i#*=}"
                shift
            ;;
            --link)
                SAI_LINK="1"
                shift
            ;;
            -h|--help)
                show_usage
                exit 0
            ;;
            *)
                echo "ERROR: Unknown option \"$i\""
                show_usage
                exit 1
            ;;
        esac
    done
}

parse_argument $@

mkdir -p ${RELPKG_DIR}/xdk

# Upack the xdk sources, do not use SAI stored inside
tar -jxf $XDK_TARBALL --strip-components=1 -C ${RELPKG_DIR}/xdk
rm -rf ${RELPKG_DIR}/xdk/plugins/SAI

# attach adapter sources
if [ "$SAI_LINK" = "0" ]; then
    tar -jxf $SAI_TARBALL --strip-components=1 -C ${RELPKG_DIR}/xdk/plugins/
    pushd ${RELPKG_DIR}/xdk/plugins/
else
    tar -jxf $SAI_TARBALL --strip-components=1 -C ${RELPKG_DIR}
    pushd ${RELPKG_DIR}/xdk/plugins && ln -s ../../SAI && popd
    pushd ${RELPKG_DIR}
fi

OCP_SAI_DIR="$(git config --file release.info --get-regexp sai.path | awk '{print $2}')"
OCP_SAI_URL="$(git config --file release.info --get-regexp sai.url | awk '{print $2}')"
OCP_SAI_COMMIT="$(git config --file release.info --get-regexp sai.commit | awk '{print $2}')"
OCP_SAI_BRANCH="$(git config --file release.info --get-regexp sai.branch | awk '{print $2}')"

# Clone the SAI headers
git clone -b $OCP_SAI_BRANCH $OCP_SAI_URL $OCP_SAI_DIR
pushd $OCP_SAI_DIR && git checkout $OCP_SAI_COMMIT && popd

popd
echo "INFO: The xdk sources with custom SAI adapter is ready"