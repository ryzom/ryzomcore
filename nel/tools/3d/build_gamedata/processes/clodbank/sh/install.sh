#!/bin/bash
rm log.log 2> /dev/null

# Install clodbank in the client data, in the "shapes/" directory

# Get the shape install directory
shape_install_directory=`cat ../../cfg/directories.cfg | grep "shape_install_directory" | sed -e 's/shape_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../../cfg/site.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Install clodbank >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install clodbank 
echo ------- 
date >> log.log
date

cp -u -p -R clodbank/. $client_directory/$shape_install_directory  2>> log.log
