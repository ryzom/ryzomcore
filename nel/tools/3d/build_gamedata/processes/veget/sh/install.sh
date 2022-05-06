#!/bin/bash
rm log.log 2> /dev/null

# Install vegets in the client data

# Get the veget install directory
veget_install_directory=`cat ../../cfg/directories.cfg | grep "veget_install_directory" | sed -e 's/veget_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../../cfg/site.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Install veget >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install veget 
echo ------- 
date >> log.log
date

cp -u -p -R veget/. $client_directory/$veget_install_directory  2>> log.log
