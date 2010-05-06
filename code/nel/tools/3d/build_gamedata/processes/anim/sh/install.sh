#!/bin/bash
rm log.log 2> /dev/null

# Install anim in the client data

# Get the anim install directory
anim_install_directory=`cat ../../cfg/directories.cfg | grep "anim_install_directory" | sed -e 's/anim_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../../cfg/site.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Install animation >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install animation 
echo ------- 
date >> log.log
date

cp -u -p -R anim/. $client_directory/$anim_install_directory  2>> log.log
