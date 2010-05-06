#!/bin/sh

FILENAME=../RyzomCompilerFlags.mk.$(hostname -s)

if [ -e $FILENAME ]
    then
    echo RYZOM_VERSION_COMPILER_FLAGS=$(cat $FILENAME)
else
    echo ERROR: File not found $FILENAME
    exit 1
fi
