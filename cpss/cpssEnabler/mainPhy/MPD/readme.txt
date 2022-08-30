how to compile MPD:
first, there are three targets:
bm - Black mode
    compiles to actual target
gm - Grey mode
    compiles to host (running machine) native, for simulation
wm - White mode
    compiles to windows, requires wine (and a few other things), for simulation

gm_sample - compile sample (executable) application
    
following will regard only to BM & GM targets:
there are 4 essential variables that must be provided in order to compile MPD,
1. CROSS_COMPILE - this is required for BM, the compiler (see appendix)
2. CROSS_LIB_PATH - location of libc matching CROSS_COMPILE


Appendix - compiler configuration
========
/swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi-gcc -v
Using built-in specs.
COLLECT_GCC=/swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi-gcc
COLLECT_LTO_WRAPPER=/nfs/pt/swdev/areas/readonly/swtools/devtools/gnueabi/arm_le/arm-none-linux-gnueabi-versions/SDK_EBU_2013_Q3.1/armv7-marvell-linux-gnueabi-softfp_i686_64K_Dev_20131002/bin/../libexec/gcc/arm-marvell-linux-gnueabi/4.6.4/lto-wrapper
Target: arm-marvell-linux-gnueabi
Configured with: /home/gccbuilder-i386/work/mgcc4.6/src/gcc-src/configure --host=i486-linux-gnu --build=i486-linux-gnu --target=arm-marvell-linux-gnueabi --prefix=/home/gccbuilder-i386/work/mgcc4.6/Release/install/armv7-marvell-linux-gnueabi-softfp_i686_64K_Dev --with-gnu-as --with-gnu-ld --enable-symvers=gnu --enable-__cxa_atexit --with-pkgversion='Linaro GCC branch-4.6.4. Marvell GCC Dev 201310-2126.3d181f66 64K MAXPAGESIZE ALIGN' --with-bugurl=mailto:mrvlgccbug@gmail.com --disable-nls --disable-libmudflap --disable-libstdcxx-pch --disable-libssp --enable-shared --enable-threads --enable-languages=c,c++ --with-sysroot=/home/gccbuilder-i386/work/mgcc4.6/Release/install/armv7-marvell-linux-gnueabi-softfp_i686_64K_Dev/arm-marvell-linux-gnueabi/libc --with-build-sysroot=/home/gccbuilder-i386/work/mgcc4.6/Release/install/armv7-marvell-linux-gnueabi-softfp_i686_64K_Dev/arm-marvell-linux-gnueabi/libc --with-gmp=/home/gccbuilder-i386/work/mgcc4.6/host_libs_marvell/i686/install --with-mpfr=/home/gccbuilder-i386/work/mgcc4.6/host_libs_marvell/i686/install --with-ppl=/home/gccbuilder-i386/work/mgcc4.6/host_libs_marvell/i686/install --with-host-libstdcxx='-static-libgcc -Wl,-Bstatic,-lstdc++,-Bdynamic -lm' --with-cloog=/home/gccbuilder-i386/work/mgcc4.6/host_libs_marvell/i686/install --enable-cloog-backend=isl --with-mpc=/home/gccbuilder-i386/work/mgcc4.6/host_libs_marvell/i686/install --enable-poison-system-directories --enable-lto --enable-libgomp --with-build-time-tools=/home/gccbuilder-i386/work/mgcc4.6/Release/install/armv7-marvell-linux-gnueabi-softfp_i686_64K_Dev/arm-marvell-linux-gnueabi/bin --enable-multiarch --with-specs='%{funwind-tables|fno-unwind-tables|mabi=*|ffreestanding|nostdlib:;:-funwind-tables}' --with-cpu=marvell-pj4 --with-float=softfp --with-fpu=vfpv3-d16 --enable-mvl-thumb2-multilibs
Thread model: posix
gcc version 4.6.4 (Linaro GCC branch-4.6.4. Marvell GCC Dev 201310-2126.3d181f66 64K MAXPAGESIZE ALIGN)


examples for make commands:
--------------------------
make gm 
make bm CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi- CROSS_LIB_PATH=/swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/arm-marvell-linux-gnueabi/libc

file build_dir_bm/obj/src/mpd.o
build_dir_bm/obj/src/mpd.o: ELF 32-bit LSB relocatable, ARM, version 1 (SYSV), not stripped
file build_dir_gm/obj/src/mpd.o
build_dir_gm/obj/src/mpd.o: ELF 64-bit LSB relocatable, x86-64, version 1 (SYSV), not stripped
