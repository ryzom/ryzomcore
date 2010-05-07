#!/bin/sh

rm -rf html/tool
WORKDIR=$(pwd)

cd ../../tool
export CURDIR=$(pwd)
cd $WORKDIR
doxygen neltools.dox -DCURDIR
