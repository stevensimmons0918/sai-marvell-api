#!/bin/bash

export CPSS=$PWD
export LUA=$PWD/lua_cli
#export SIMFS_PATH=$CPSS/simulation/simGeneral/h/asicSimulation/SEmbedded

export DATA_PATH=$CPSS/mainLuaWrapper/data/dxCh
rm -f $DATA_PATH/*
python $CPSS/mainLuaWrapper/scripts/cmdFS_RAM_embed_files.py \
	-o $DATA_PATH/cmdFS_RAM_embed_files.c \
	-z .lua -z .xml -S 1000000 \
	-X px -X examples -X dxCh/examples \
	-D $LUA/scripts
python $CPSS/mainLuaWrapper/scripts/cmdFS_RAM_embed_files.py \
	-o $DATA_PATH/cmdFS_RAM_lua_cli_examples.c \
	-z .lua -z .xml -z Eeprom.txt -S 1000000 \
	-E _EXAMPLES \
	-I examples -I dxCh/examples \
	-D $LUA/scripts

export DATA_PATH=$CPSS/mainLuaWrapper/data/px
rm -f $DATA_PATH/*
python $CPSS/mainLuaWrapper/scripts/cmdFS_RAM_embed_files.py \
	-o $DATA_PATH/cmdFS_RAM_embed_files.c \
	-z .lua -z .xml -S 1000000 \
	-X dxCh -X examples -X px/examples \
	-D $LUA/scripts
python $CPSS/mainLuaWrapper/scripts/cmdFS_RAM_embed_files.py \
	-o $DATA_PATH/cmdFS_RAM_lua_cli_examples.c \
	-z .lua -z .xml -S 1000000 \
	-E _EXAMPLES \
	-I examples -I px/examples \
	-D $LUA/scripts


export DATA_PATH=$CPSS/mainLuaWrapper/data/dxpx
rm -f $DATA_PATH/*
python $CPSS/mainLuaWrapper/scripts/cmdFS_RAM_embed_files.py \
	-o $DATA_PATH/cmdFS_RAM_embed_files.c \
	-z .lua -z .xml -S 1000000 \
	-X examples -X px/examples  -X dxCh/examples\
	-D $LUA/scripts
python $CPSS/mainLuaWrapper/scripts/cmdFS_RAM_embed_files.py \
	-o $DATA_PATH/cmdFS_RAM_lua_cli_examples.c \
	-z .lua -z .xml -z Eeprom.txt -S 1000000 \
	-E _EXAMPLES \
	-I examples -I px/examples -I dxCh/examples \
	-D $LUA/scripts

#cd $SIMFS_PATH
#python $CPSS/simulation/tools/simEmbeddedFS/linux/simFS_embed_files.py $CPSS/simulation
