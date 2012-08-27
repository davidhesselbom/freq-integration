#!/bin/bash
set -e
echo 1
# test that build output exists
outputname=argcheck
if [ -n "$(uname -s | grep MINGW32_NT)" ]; then
    outputname=release/argcheck.exe
fi
echo 2
[ -f $outputname ]
echo 3
if $outputname --hahaha ; then 
	false
	echo 4
else 
	true
	echo 5
fi
echo 5