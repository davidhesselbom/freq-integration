#!/bin/bash
set -e

# test that build output exists
outputname=libcommon.a
if [ -n "$(uname -s | grep MINGW32_NT)" ]; then
    outputname=release/common.lib
fi

[ -f $outputname ]
