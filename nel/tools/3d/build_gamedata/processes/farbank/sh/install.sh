#!/bin/bash
rm log.log 2> /dev/null

# Install the farbank in the client data

# Get the bank install directory
bank_install_directory=`cat ../../cfg/directories.cfg | grep "bank_install_directory" | sed -e 's/bank_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../../cfg/site.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Install farbank >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install farbank 
echo ------- 
date >> log.log
date

cp -u -p -R farbank/. $client_directory/$bank_install_directory  2>> log.log
