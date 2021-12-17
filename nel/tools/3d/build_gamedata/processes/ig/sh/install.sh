#!/bin/bash
rm log.log 2> /dev/null

# Install ig in the client data

# Get the ig install directory
ig_install_directory=`cat ../../cfg/directories.cfg | grep "ig_install_directory" | sed -e 's/ig_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../../cfg/site.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the landscape name
landscape_name=`cat ../../cfg/config.cfg | grep "landscape_name" | sed -e 's/landscape_name//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Install Ig >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install Ig 
echo ------- 
date >> log.log
date

cp -u -p "$landscape_name"_ig.txt $client_directory/$ig_install_directory  2>> log.log

# Do not copy ig_land, because zone process will copy zone ig lighted versions into client directory.
#cp -u -p ig_land/*.[iI][gG] $client_directory/$ig_install_directory  2>> log.log
# Do not copy ig_other, because ig_light process will copy ig lighted versions into client directory.
#cp -u -p ig_other/*.[iI][gG] $client_directory/$ig_install_directory  2>> log.log
