#!/usr/bin/env bash

#a script to setup environment variables for correct run of xpSaiApp
#run this in root of unpacked XDK on the target HW
#to run use the following syntax:
#source ./env_sai.sh
#Note: in case you have the crb2/other platform, please update the path below

export LD_LIBRARY_PATH=`pwd`/dist/
export PLATFORM_CONFIG_PATH=`pwd`/platform/crb1-rev1/
export XP_ROOT=`pwd`
export AAPL_ROM_PATH=`pwd`

echo "env variables exported"
