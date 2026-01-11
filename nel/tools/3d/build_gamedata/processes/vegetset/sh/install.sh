#!/bin/bash
rm log.log 2> /dev/null

# Install the vegetable set in the client data

# Get the vegetset install directory
vegetset_install_directory=`cat ../../cfg/directories.cfg | grep "vegetset_install_directory" | sed -e 's/vegetset_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../../cfg/site.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Install vegetset >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install vegetset 
echo -------
date >> log.log
date

cp -u -p -R vegetset/. $client_directory/$vegetset_install_directory  2>> log.log
