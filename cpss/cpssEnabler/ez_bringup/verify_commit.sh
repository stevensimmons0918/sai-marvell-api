#!/bin/bash -e

make clean
make -C pdl -f Makefile_bm CFLAGS=-Werror all CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi-
make -f Makefile_bm CFLAGS=-Werror all
make -f Makefile_wm CFLAGS=/WX all



