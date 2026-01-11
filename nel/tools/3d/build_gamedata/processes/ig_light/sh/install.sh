#!/bin/bash
rm log.log 2> /dev/null

# Install ig in the client data

# Get the ig install directory
ig_install_directory=`cat ../../cfg/directories.cfg | grep "ig_install_directory" | sed -e 's/ig_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../../cfg/site.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Install Lighted Ig >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install Lighted Ig 
echo ------- 
date >> log.log
date

cp -u -p -R ig_other_lighted/. $client_directory/$ig_install_directory  2>> log.log

