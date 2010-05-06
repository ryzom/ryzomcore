#!/bin/bash
rm log.log 2> /dev/null

# Install the particule system in the client data

# Get the ps install directory
ps_install_directory=`cat ../../cfg/directories.cfg | grep "ps_install_directory" | sed -e 's/ps_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../../cfg/site.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Install ps >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install ps 
echo -------
date >> log.log
date

cp -u -p -R ps/. $client_directory/$ps_install_directory  2>> log.log
