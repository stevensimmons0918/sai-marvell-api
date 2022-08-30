#!/bin/bash

############################################################
# This script is used to correct the syntex error in .gen.i
# files. It will verify all .gen.i files for correct syntex,
# if any syntex error found then correct that error.
#
# This script has been added due to the error occur in one
# of the .gen.i file for Octeon Thunderx.
############################################################

IFS=''

#Find the list of all .gen.i files and save into file_list.txt
$(find ../ -iname "*.gen.i*" > file_list.txt)

#Remove if sdk_py_log.txt file is already available
if [ -f "sdk_py_log.txt" ] ; then
    rm -rf "sdk_py_log.txt"
fi
echo "`date` : sdk_py target syntax error checking process has been started" >> sdk_py_log.txt

#Read the filename in the loop
while read -r filename || [[ -n "$filename" ]] ; do

    if [ -f "$filename" ] ; then
        #Remove if temp.txt file is already available
        if [ -f "temp.txt" ] ; then
            rm -rf "temp.txt"
        fi

        while read  -r line ; do
            if [[ $line = *operator* ]]; then
                line1=$line
                read line
                if [[ $line = *bool* ]]; then
                    line2=$line
                    read line
                    if [[ $line = *const* ]]; then
                        echo "syntax error occurred in file $filename" >> sdk_py_log.txt
                        echo "$line1 $line2 $line" >> temp.txt
                    else
                        echo $line1 >> temp.txt
                        echo $line2 >> temp.txt
                        echo $line >> temp.txt
                    fi
                else
                    echo $line1 >> temp.txt
                    echo $line >> temp.txt
                fi
            else
                echo $line >> temp.txt
            fi
        done < "$filename"

        mv temp.txt $filename
        echo "Checked for syntax error in file $filename" >> sdk_py_log.txt
    fi

done < file_list.txt
echo "`date` : sdk_py target syntax error checking process has been done" >> sdk_py_log.txt

#Delete the list of .gen.i files
$(rm -rf file_list.txt)
