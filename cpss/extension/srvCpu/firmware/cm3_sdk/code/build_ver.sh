#!/bin/bash

# Prepares a release of CM3 SDK, that includes:
# 1. CM3 SDK FW
# 2. CM3 tool
# 3. Virtual uart tool

# Assumes CM3 SDK is at subir extension/srvCpu/firmware/cm3_sdk/code
SDK_PATH=extension/srvCpu/firmware/cm3_sdk/code
CM3_TOOL_PATH=extension/srvCpu/firmware/cm3_sdk/tool/cm3_tool
VUART_PATH=extension/srvCpu/firmware/cm3_sdk/tool/virtual_uart

base_dir=`realpath $PWD/../../../../..`
root_dir_len=$((${#base_dir}+1))
#echo $root_dir_len

# Create a list of files from CM3_SDK Compilation
make -s clean
export CFLAGS=-H; make SHM_UART=y SHM_UART_BASE=0x1f000 SHM_UART_SIZE=256 QSPI=y SDK_DEBUG=y &>tmp1.txt
make -s clean
export CFLAGS=-H; make &>tmp2.txt
# union the lists to have a new list that includes files from both cases of ifdef compilation flag
comm --nocheck-order tmp1.txt tmp2.txt > tmp.txt
# remove tabs from union list
sed -i -e 's/[ \t]*//' tmp.txt
rm tmp1.txt tmp2.txt


# Generates a list of all included .h files
# grep . => remove empty lines
list=`grep -v "swtools" tmp.txt | grep -v "Multiple" | cut -d " " -f 2- | awk '/\.h/ || /\.c/' | sort -u | awk '{if($1 == "CC") print $2; else print $0}'| grep . | xargs realpath`
mkdir cpss
#echo $list

cd cpss
while read -r line; do
    dst=`dirname ${line:root_dir_len}`
    #echo $dst
    #exit
    mkdir -p $dst
    cp $line ./$dst
done <<< "$list"
#exit

cd -
cp Makefile RTOSDemo-cm3.ld cpss/$SDK_PATH
rm tmp.txt

echo -e "\nNow building version...\n"

cd cpss/$SDK_PATH
export CFLAGS= ; make SHM_UART=y SHM_UART_BASE=0x1f000 SHM_UART_SIZE=256 QSPI=y SDK_DEBUG=y -s
if [ $? -ne 0 ]    # last result - of make
then
    echo
    echo -e "\e[31m Building packed version failed !!!\e[39m"
    exit
fi

make clean
cd -

# Generates a list of all included .h files also for cm3 tool
cd $base_dir/$CM3_TOOL_PATH
make clean
export CFLAGS=-H; make  &>tmp.txt

list=`grep -v "swtools" tmp.txt | grep -v "Multiple" | cut -d " " -f 2- | awk '/\.h/ || /\.c/' | sort -u | awk '{if($1 == "CC") print $2; else print $0}'| grep . | xargs realpath`
#echo $list
#exit
rm tmp.txt
cd -
cd cpss
while read -r line; do
    dst=`dirname ${line:root_dir_len}`
    #echo $dst
    mkdir -p $dst
    cp $line ./$dst
done <<< "$list"

# copy also cm3-tool & virtual-uart
mkdir -p $CM3_TOOL_PATH $VUART_PATH
cp $base_dir/$CM3_TOOL_PATH/* $CM3_TOOL_PATH/
cp $base_dir/$VUART_PATH/*    $VUART_PATH/

# delete BINs
find . -name "*.o" -o -name "*.d" -o -name "*.bin" -o -name "*.map" -o -name "*.elf" -o -name "*.orig" | xargs rm
cd -

tar czf cm3_sdk.tar.gz cpss
rm -rf cpss/
echo -e "\e[32m cm3_sdk.tar.gz\e[39m is ready!"

