#!/bin/bash
# A script for manual rebuild of generated files

# Any subsequent commands which fail will cause the shell script to exit immediately
set -e

# Evaluate CPSS path from script location
C=$(realpath $(dirname $0)/../../)

L=$C/mainLuaWrapper
S=$L/scripts
D=$L/data


# create family related type wrappers
# all_dxCh_lua_c_type_wrappers.c:
python $S/make_type_wrapper.py \
	-i $D/type_info_file_dx -p $S/already_implemented.lst -X $S/excludelist \
	-C $C -M $S/lua_C_mapping -N -T -F dxCh\
	-o $L/src/dxCh/all_dxCh_lua_c_type_wrappers.c

# all_px_lua_c_type_wrappers.c:
python $S/make_type_wrapper.py \
	-i $D/type_info_file_px -p $S/already_implemented.lst -X $S/excludelist \
	-C $C -M $S/lua_C_mapping -N -F px\
	-o $L/src/px/all_px_lua_c_type_wrappers.c

# all_dxpx_lua_c_type_wrappers.c:
#python $S/make_type_wrapper.py \
#	-i $D/type_info_file_dxpx -p $S/already_implemented.lst -X $S/excludelist \
#	-C $C -M $S/lua_C_mapping -N -T -F px -F dxCh\
#	-o $L/src/dxpx/all_dxpx_lua_c_type_wrappers.c


# create common type wrappers
# all_common_lua_c_type_wrappers.c:
python $S/make_type_wrapper.py \
	-i $D/type_info_file_dx \
	-p $S/already_implemented.lst -X $S/excludelist \
	-C $C -M $S/lua_C_mapping -N -T -F NONE \
        -I common \
	-o $L/src/generic/all_common_lua_c_type_wrappers.c

# all_common_noppdrv_lua_c_type_wrappers.c:
python $S/make_type_wrapper.py \
	-i $D/type_info_file_px \
	-p $S/already_implemented.lst -X $S/excludelist \
	-C $C -M $S/lua_C_mapping -N -F NONE \
	-I common -I mainPxDrv \
	-o $L/src/generic/all_common_noppdrv_lua_c_type_wrappers.c

# all_common_dxpx_lua_c_type_wrappers.c:
python $S/make_type_wrapper.py \
	-i $D/type_info_file_dxpx \
	-p $S/already_implemented.lst -X $S/excludelist \
	-C $C -M $S/lua_C_mapping -N -F NONE \
	-I common -I mainPxDrv -I mainPpDrv \
	-o $L/src/generic/all_common_dxpx_lua_c_type_wrappers.c


# cpssAPI.xml
X=$D/dxCh_xml
#Create integrated DX cpssAPI.xml
python $S/gen_xml_funcdescr.py -i $D/type_info_file_dx -A $D/hiddenAPIs -C $C -F dxCh \
	-X $S/excludelist -o $X/cpssAPI.xml -B $X/bad_s.xml -c \
	-M $S/manual_s.xml -m $S/lua_C_mapping
# cpssAPI_xml_embed_file.c
python $S/cmdFS_RAM_embed_files.py \
    -E _XML -z .xml \
    -S 10000000 \
    -o $X/cpssAPI_xml_embed_file.c \
    -F $X/cpssAPI.xml

X=$D/px_xml
#Create integrated PX cpssAPI.xml
python $S/gen_xml_funcdescr.py -i $D/type_info_file_px -A $D/hiddenAPIs -C $C -F px \
	-X $S/excludelist -o $X/cpssAPI.xml -B $X/bad_s.xml -c \
	-m $S/lua_C_mapping
# cpssAPI_xml_embed_file.c
python $S/cmdFS_RAM_embed_files.py \
    -E _XML -z .xml \
    -S 10000000 \
    -o $X/cpssAPI_xml_embed_file.c \
    -F $X/cpssAPI.xml

X=$D/dxpx_xml
#Create integrated DXPX cpssAPI.xml
python $S/gen_xml_funcdescr.py -i $D/type_info_file_dxpx -A $D/hiddenAPIs -C $C -F px -F dxCh \
	-X $S/excludelist -o $X/cpssAPI.xml -B $X/bad_s.xml -c \
	-M $S/manual_s.xml -m $S/lua_C_mapping
# cpssAPI_xml_embed_file.c
python $S/cmdFS_RAM_embed_files.py \
    -E _XML -z .xml \
    -S 10000000 \
    -o $X/cpssAPI_xml_embed_file.c \
    -F $X/cpssAPI.xml
