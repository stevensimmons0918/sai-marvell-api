#!/bin/bash
grep -wr typedef $1  | awk '{ print "\t\t\tenum \"" $2 "\" { description \"" $2 "\" ;}" }'

