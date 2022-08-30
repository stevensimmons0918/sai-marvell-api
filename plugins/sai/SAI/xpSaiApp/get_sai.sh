#!/bin/bash

#a script to retrieve the appropriate SAI version from github for XDK release package
#Please execute this script from the xpSaiApp folder

SAI_PATH=../xpSai/

# Download SAI sources
cd $SAI_PATH
if [ ! -d "sai" ]; then
  rm -rf sai
  git clone https://github.com/opencomputeproject/SAI sai
  cd sai
  git reset --hard ecad653aa13dfdde873d2cc829ff32d7c176482f
fi
