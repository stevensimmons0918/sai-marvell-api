#!/usr/bin/env bash
# parameters: 

module_file=$1
home_romfs=$2

test -d $home_romfs || exit 0

################################################
#
# 1. Copy module to $home_romfs/lib/modules
#
################################################
echo "	cp $module_file $home_romfs/lib/modules"
cp $module_file $home_romfs/lib/modules


################################################
#
# 2. Check if device node exists.
#    Create node if it not exists yet
#
################################################
if [ -f $home_romfs/../rootfs_deviceMap ]; then
	dm=$home_romfs/../rootfs_deviceMap
	grep -q '^\/dev\/mvKernelExt' $dm >/dev/null 2>&1
	if [ $? -ne 0 ]; then
		echo "/dev/mvKernelExt c 640 0 0 254   1 0 0 -" >>$dm
	fi
else
    cd $home_romfs/dev
    if [ \! -c mvKernelExt ]
    then
        echo pwd=`pwd`
        echo mknod mvKernelExt c 254 1
        sudo mknod mvKernelExt c 254 1
    fi
fi


################################################
#
# 3. Check if $home_romfs/etc/init.sh loads module
#    Add module load commands if necessary
#
################################################
cd $home_romfs/etc
grep -q $module_file init.sh || {
	echo "patching init.sh"
	sed '/telnetd/ a\
\
 if test -e /lib/modules/'$module_file'\
 then\
   insmod /lib/modules/'$module_file'\
 fi' init.sh >init.sh.$$
	cat init.sh.$$ >init.sh
	rm init.sh.$$
}
