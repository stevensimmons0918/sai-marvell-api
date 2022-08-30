#!/bin/bash

SAI_PATH=`pwd`/..

cd $SAI_PATH/xpSai

ADAPTER_ATTR_NUM=($(grep -rn "\scase SAI_.*_ATTR_" . | sed 's/.*[0-9]:\s*//' | sort | uniq | wc -l))

ADAPTER_API_NUM=($(grep -rn "= xpSai" . | grep _xpSai | sed 's/.*[0-9]:\s*//' | sort | uniq | wc -l))

cd $SAI_PATH/xpSai/sai/inc

HEADERS_ATTR_NUM=($(grep -rn "SAI_.*_ATTR_" . | sed 's/.*[0-9]:\s*//' | grep "^SAI_" | sed 's/[;,:\s ].*//' | sed 's/\].*$//' | sort | uniq | wc -l))

HEADERS_API_NUM=($(grep -rn "sai_.*_fn" . | grep -v "typedef" | sed 's/.*[0-9]:\s*//' | awk '{print $2}' | sort | uniq | wc -l))

cd $SAI_PATH/xpSaiApp

APP_ATTR_NUM=($(grep -rn "SAI_.*_ATTR_" . --include=*.c --include=*.cpp | sed 's/.*[0-9]:\s*//' | sed 's/.*SAI_/SAI_/' | sed 's/[;,:\s ].*//' | sed 's/\].*$//' | sort | uniq | grep -v "_COUNT$" | wc -l))

APP_API_NUM=($(grep -rn "Api->" . | sed 's/.*[0-9]:\s*//' | sed 's/.*xpSai/xpSai/g' | sed 's/(.*//' | sort | uniq | wc -l))

echo "###############################################"
echo "################# SAI report ##################"
echo "###############################################"
printf '%-20s %-20s %-20s \n' "Name" "Attributes" "APIs"
printf '%-20s %-20s %-20s \n' "-------" "----------" "----"
printf '%-20s %-20s %-20s \n' "Headers" $HEADERS_ATTR_NUM $HEADERS_API_NUM
printf '%-20s %-20s %-20s \n' "Adapter" $ADAPTER_ATTR_NUM $ADAPTER_API_NUM
printf '%-20s %-20s %-20s \n' "SaiApp" $APP_ATTR_NUM $APP_API_NUM

echo "###############################################"



