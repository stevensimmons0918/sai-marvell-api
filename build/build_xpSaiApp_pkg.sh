#!/bin/bash
#
# Steps to use:
# cd build/
# ./build_xpSaiApp_pkg.sh
#       OR
# ./build_xpSaiApp_pkg.sh <filename>.tgz 
#
# Resulted TAR file will be placed in dist/
#

cd ..

if [ -z "$1" ]; then
    TAR=dist/xpSaiApp_$(date +%Y-%m-%d).tgz
else
    TAR=dist/$1
fi

declare -a F
if [ -f build/objs/a385/cpss/appDemo ]
then
    F+=(build/objs/a385/cpss/appDemo)
fi
if [ -f build/objs/x86_64/cpss/appDemo ]
then
    F+=(build/objs/x86_64/cpss/appDemo)
fi
if [ -f build/objs/aarch64/cpss/appDemo ]
then
    F+=(build/objs/aarch64/cpss/appDemo)
fi
if [ -f build/objs/native/cpss/appDemoSim ]
then
    F+=(build/objs/native/cpss/appDemoSim)
fi

tar cvzf ${TAR}         \
    --exclude=".nfs*"     \
    --exclude="*.o"    \
    --exclude="*.txt"   \
    --exclude="*.h"     \
    --exclude="*.cpp"   \
    --exclude="*.mk"    \
    --exclude="*.i"     \
    --exclude="*.d"     \
    --exclude="*.a"     \
    --exclude="*.doc"   \
    --exclude="*.docx"  \
    --exclude="*.hpp"  \
    --exclude="*.c"  \
    --exclude="*.txt"  \
    --exclude="*.git*"  \
    --exclude="*.pcap"  \
    --exclude="*.xlsx"  \
    --exclude="*.*pData"  \
    --exclude="*.xml"  \
    --exclude="*.md"  \
    --exclude="*.js"  \
    --exclude="*.html"  \
    --exclude="xpsApp"  \
    --exclude="xpSaiApp_dbg"  \
    --exclude="xpSaiApp_symbols"  \
    --exclude="*.tgz"  \
    --exclude="*.tar*"  \
    --exclude=$TAR  \
    --dereference \
    --transform 's,^,xpSaiApp/,' \
    dist/ cli/ plugins/sai/sai-src/saiShell/ fwFiles/ \
    ${F[@]}


ls -lh ${TAR}
cd -
