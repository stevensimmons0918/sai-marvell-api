export CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/arm-none-linux-gnueabi-versions/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
export SYSROOT=/swtools/devtools/gnueabi/arm_le/arm-none-linux-gnueabi-versions/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/ELDK
export PATH=/swtools/devtools/gnueabi/arm_le/arm-none-linux-gnueabi-versions/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/bin:$PATH
./maker $@
