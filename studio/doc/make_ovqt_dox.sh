#!/bin/sh

rm -rf html/ovqt
WORKDIR=$(pwd)

cd ..
export CURDIR=$(pwd)
cd $WORKDIR
doxygen ovqt.dox -DCURDIR
