#!/bin/sh

if [ "$1" = "" ]
then
    echo
    echo USAGE: $0 command_line
    echo
    echo example:
    echo   $0 echo hello world
    echo   displays 'hello world' repeatedly, delaying 3 seconds between repeats
    echo
    exit
fi

while true
do
  sleep 3
  eval $*
done
