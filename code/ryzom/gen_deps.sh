#!/bin/bash

command=''
sources=''
phase=0
for arg in $*
do
    if [ "$phase" == 0 ] && [ "$arg" == '--' ]
    then
        phase=1
    elif [ "$phase" == 0 ]
    then
        command="$command $arg"
    elif [ "$phase" == 1 ]
    then
        sources="$sources $arg"
    fi
done

for src in $sources
do
    obj=`echo $src | sed -e 's/.cpp$/.o/'`
    $command -MT $obj -M $src
done
