#!/bin/sh

rm -rf html/nelns
WORKDIR=$(pwd)

cd ../../nelns
export CURDIR=$(pwd)
cd $WORKDIR
doxygen nelns.dox -DCURDIR
