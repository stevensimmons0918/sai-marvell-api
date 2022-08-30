# #!/bin/bash

##############################################################
#                                                            #
#      Building root filesystem for embedded Linux system    #
#                                                            #
##############################################################
#
## functions
#

set -e

echo ""
if [ $# -lt 3 ] || [ ! "$1" != "--help" ]
then
  echo "Usage: crfs.sh <ROOTFS_PATH> <CROSS_COMPILE> <BUSYBOX_PATH>"
  echo " Create root filesystem for ARM CPU in path given by <ROOTFS_PATH>."
  echo ""
  echo "Example: "
  echo " ./crfs.sh /tftpboot/rootfs /usr/local/tools/arm-none-linux-gnueabi_SDK-3.2/bin/arm-mv5sft-linux-gnueabi- ~/projects/axp/busybox-1.21.1.tar.bz2"
  echo ""
  exit 0
fi

## main
#

#set default arguments:

gnueabi=`dirname "$2" | xargs dirname`
default_gdbserver_app=$(find  $gnueabi/ -name gdbserver | head -n1)

user_path=`pwd`
echo $user_path

rootfs_path=$1
cd $user_path
if [ -d $1 ]
then
	echo "File system root: $rootfs_path"
else
	echo "Creating file system root: $rootfs_path"
	mkdir -p $rootfs_path
	if [ ! -d $1 ]
	then
		echo "Failed! Please check rootfs path $1."
		exit
	fi
fi

if [ -e $gnueabi ]
then
	echo "GNUEABI path: $gnueabi"
else
	echo "GNUEABI not found in $2. Exiting..."
	echo ""
	exit 1
fi


busybox_path=$3

gdbserver_file=$default_gdbserver_app

if [ ! -e $gdbserver_file ]
then
	echo "gdbserver file is missing. Exiting..."
	echo
	exit 1
fi

d1=`dirname ${gdbserver_file}`
cd ${d1}
d1=`pwd`
gdbserver_file=${d1}/`basename ${gdbserver_file}`

cd $user_path
cd $rootfs_path

tar xf $busybox_path
busybox_folder=`ls -d busybox*`


#create file structure
#---------------------
mkdir bin
mkdir dev
mkdir sbin
mkdir root
mkdir home
mkdir home/user
mkdir mnt
mkdir mnt/flash
mkdir mnt/nfs
mkdir etc
mkdir proc
mkdir sys
mkdir tmp
mkdir lib
mkdir lib/modules
mkdir usr
mkdir usr/lib
mkdir usr/bin
mkdir usr/local
mkdir usr/sbin
mkdir usr/share
mkdir var
mkdir var/lib
mkdir var/lock
mkdir var/log
mkdir var/run
mkdir var/tmp

echo "Building libraries"
# copy libraries
# --------------
cd $user_path
cd $rootfs_path

cd usr/bin
usr_bin_path=`pwd`

cd ../../lib
lib_path=`pwd`

gnueabi_lib=$(find  $gnueabi/ -name libgcc_s.so | head -n1 | xargs dirname)
if [ -d ${gnueabi_lib}c/lib ]; then
	gnueabi_lib="${gnueabi_lib}c/lib"
fi
gnueabi_prefix=$2

cd  $gnueabi_lib
cp -d libresolv* libstd* libc-* libc.* libm*  ld-* libcrypt* $lib_path
cp -d libthread_db*  libpthread* $lib_path
cp -d libgcc* $lib_path
cp -d librt*  libdl* $lib_path

cd $lib_path
rm -f *orig*

cp $gdbserver_file $usr_bin_path/gdbserver
${gnueabi_prefix}strip ${usr_bin_path}/gdbserver

echo "Creating etc files"
# create init files
# -----------------
cd $user_path
cd $rootfs_path

# creating /root/.profile

cat << EOF > ./root/.profile
alias ll='ls -la'
EOF

cd etc

# creating passwd
echo -e "root::0:0:root:/root:/bin/sh\n\nuser::500:500:Linux User,,,:/home/user:/bin/sh\n" >./passwd

# creating group
echo -e "root:x:1:root\nuser:x:500:\n" >./group
# creating inittab
echo -e "\n# autoexec\nttyS0::respawn:/etc/init.sh\n#::once:/etc/init.sh\n\n# Stuff to do when restarting the init process\n::restart:/sbin/init\n" >./inittab

# creating motd
cat << EOF > ./motd


Welcome to Embedded Linux
           _  _
          | ||_|
          | | _ ____  _   _  _  _
          | || |  _ \\| | | |\\ \\/ /
          | || | | | | |_| |/    \\
          |_||_|_| |_|\\____|\\_/\\_/

          On Marvell's ARM board

For further information on the Marvell products check:
http://www.marvell.com/

toochain=gnueabi

Enjoy!

EOF


# creating welcome file for telnet
cat << EOF > ./welcome


Welcome to Embedded Linux Telnet
           _  _
          | ||_|
          | | _ ____  _   _  _  _
          | || |  _ \\\\| | | |\\\\ \\\\/ /
          | || | | | | |_| |/    \\\\
          |_||_|_| |_|\\\\____|\\\\_/\\\\_/

          On Marvell's ARM board

toochain=gnueabi

For further information on the Marvell products check:
http://www.marvell.com/
EOF

current_date=$(date +%m%d%I%M%Y)
current_date1=$(date)


# creating README.txt
cat << EOF > ./README.txt

File system building information
--------------------------------

build_date = ${current_date1}

sdk_prefix = ${gnueabi}

lib        = \${sdk_prefix}/arm-none-linux-gnueabi/sys-root/lib

cmd_prefix = \${sdk_prefix}/bin/arm-none-linux-gnueabi-

gdbserver  = ${gdbserver_file}

EOF

# creating init.sh

cat << EOF > ./init.sh
#!/bin/sh
if test -e /proc/version
then
 echo
else

hostname MARVELL_LINUX
HOME=/root

#create dev folders
mkdir /dev/pts
mkdir /dev/shm

mount -t proc   proc /proc
mount -t sysfs  none /sys
# Performs extra mounts from fstab
mount -a

# Create tmp dir for CDB files
#mkdir /mnt/flash/tmp

echo /sbin/mdev > /proc/sys/kernel/hotplug
mdev -s

# need to check if console device was created for ramfs
# mknod -m 622 console c 5 1

cd dev/

# make block devices
if [ ! -b loop0     ]; then mknod loop0     b  7 0; fi
if [ ! -b loop1     ]; then mknod loop1     b  7 1; fi
if [ ! -b ram0      ]; then mknod ram0      b  1 0; fi
if [ ! -b ram1      ]; then mknod ram1      b  1 1; fi
if [ ! -b mtdblock0 ]; then mknod mtdblock0 b 31 0; fi
if [ ! -b mtdblock1 ]; then mknod mtdblock1 b 31 1; fi

# make character devices
if [ ! -c mem ]; then mknod mem    c   1   1; fi
if [ ! -c null ]; then mknod null   c   1   3; fi
if [ ! -c zero ]; then mknod zero   c   1   5; fi
if [ ! -c random ]; then mknod random c   1   8; fi
if [ ! -c ptyp0 ]; then mknod ptyp0  c   2   0; fi
if [ ! -c ptyp1 ]; then mknod ptyp1  c   2   1; fi
if [ ! -c ptyp2 ]; then mknod ptyp2  c   2   2; fi
if [ ! -c ttyp0 ]; then mknod ttyp0  c   3   0; fi
if [ ! -c ttyp1 ]; then mknod ttyp1  c   3   1; fi
if [ ! -c ttyp2 ]; then mknod ttyp2  c   3   2; fi
if [ ! -c tty0 ]; then mknod tty0   c   4   0; fi
if [ ! -c ttyS0 ]; then mknod ttyS0  c   4  64; fi
if [ ! -c ptmx ]; then mknod ptmx   c   5   2; fi
if [ ! -c mtd0 ]; then mknod mtd0   c  90   0 ; fi
if [ ! -c mtd0ro ]; then mknod mtd0ro c  90   1; fi
if [ ! -c mtd1 ]; then mknod mtd1   c  90   2; fi
if [ ! -c mvPP ]; then mknod mvPP   c 244   0; fi

#mknod mvROS  c 250   0

# create links
if [ ! -L console ]; then ln -s ttyS0 console; fi
if [ ! -L tty ]; then ln -s ttyS0 tty; fi
if [ ! -L ttys0 ]; then ln -s ttyS0 ttys0; fi
if [ ! -L tty5 ]; then ln -s ttyS0 tty5; fi
if [ ! -L tty1 ]; then ln -s ttyp1 tty1; fi
if [ ! -L mtd ]; then ln -s mtd0  mtd; fi

cd /
/usr/sbin/telnetd -l /bin/sh -f /etc/welcome


# load all kernel modules
kernel_ver=`uname -r`
#create modules.dep file to make modinfo command run
mkdir -p /lib/modules/
touch /lib/modules/modules.dep
for module_f in /lib/modules/${kernel_ver%%-*}/*.ko /lib/modules/*.ko
do
  if [ -e $module_f ]
  then
    insmod $module_f
  fi
done

date $current_date

# Start the network interface
/sbin/ifconfig lo 127.0.0.1

rm -f /tmp/tasks

fi

# print logo
clear
uname -nrsv
cat /etc/motd

export ENV=/root/.profile

#creation timestamp print
echo -n "creation date:"
cat /usr/local/image_date

# starting boot and ros application
# /usr/bin/appDemo

exec /bin/sh
EOF

chmod 777 init.sh

# creating fstab
cat << EOF > ./fstab
none                    /proc                   proc    defaults        0 0
none                    /sys                    sysfs   defaults        0 0
none                    /dev/shm                tmpfs   size=64M        0 0
none                    /dev/pts                devpts  mode=0622       0 0
EOF

# creating timestamp
cd ..
date > ./usr/local/image_date

#
cd $user_path
cd $rootfs_path/$busybox_folder
cp $user_path/tools/build/rootfs/config_busybox .config

# make and install Busybox
echo -e "Compiling Busybox application. This process may take several minutes.\nPlease wait...\n\n"
make CROSS_COMPILE=$gnueabi_prefix CONFIG_PREFIX=../. CONFIG_EXTRA_LDLIBS=resolv all -j 16
make CROSS_COMPILE=$gnueabi_prefix CONFIG_PREFIX=../. install

echo -e "\nCompilation completed.\n"

# remove Busybox sources
cd ..
rm -rf $busybox_folder

echo ""
echo "Filesystem created successfuly"
echo ""





