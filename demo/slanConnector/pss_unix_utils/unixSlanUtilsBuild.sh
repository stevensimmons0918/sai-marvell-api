#!/usr/bin/env bash

######################################################################
# unixSlanUtilsBuild.sh
#
# DESCRIPTION:
#       Bourne shell script to build slan Linux and FreeBSD utilities :
#         slanLib - used by appDemoSim and ethernet driver 
#         ethernet driver  - provide traffic from ethernet port to connector utility
#         connector - provides traffci from SmbSim or ethernet port to appDemo
#
# INPUTS:
#	none
#
# OUTPUTS:
#       none
#
# RETURNS:
#       none
#
# COMMENTS:
#	none
#
# FILE REVISION NUMBER:
#       $Revision: 1 $
#####################################################################

OBJ_DIR=${1}/
CURR_OS=`uname`

if [ "${CURR_OS}" = "FreeBSD" -a "${OBJ_DIR}" = "/" ] ; then
  OBJ_DIR=/tmp/objs/
  echo "OBJ_DIR=/tmp/objs/"
elif [ "${CURR_OS}" = "Linux" ] ; then
  OBJ_DIR=
fi

gmake OBJ_DIR=${OBJ_DIR} all

## end ##
