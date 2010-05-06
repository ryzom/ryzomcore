#!/bin/bash
rm log.log 2> /dev/null

# Install the displace in the client data

# Get the displace install directory
displace_install_directory=`cat ../../cfg/directories.cfg | grep "displace_install_directory" | sed -e 's/displace_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../../cfg/site.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Install displace >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install displace 
echo ------- 
date >> log.log
date

cp -u -p -R tga/. $client_directory/$displace_install_directory  2>> log.log
