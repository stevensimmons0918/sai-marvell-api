#!/usr/bin/env bash
set -x

# Build sai
cd build/
./maker cleanall
./maker CPSS_DEBUG_ON:=D_ON XDK_ENABLE_O2=0 cpss saiPreproc xpSaiApp
./maker CPSS_DEBUG_ON:=D_ON XDK_ENABLE_O2=0 saiShell_py
cd ../demo/slanConnector/pss_unix_utils/
ls -l slanConnector_Linux
make
