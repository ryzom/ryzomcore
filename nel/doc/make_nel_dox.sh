#!/bin/sh

rm -rf html/nel
WORKDIR=$(pwd)

cd ..
export CURDIR=$(pwd)
cd $WORKDIR
doxygen nel.dox -DCURDIR
