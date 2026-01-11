#!/bin/bash
rm log.log 2> /dev/null

# Install the fonts in the client data

# Get the fonts install directory
fonts_install_directory=`cat ../../cfg/directories.cfg | grep "fonts_install_directory" | sed -e 's/fonts_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../../cfg/site.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Install fonts >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install fonts
echo -------
date >> log.log
date

cp -u -p -R fonts/. $client_directory/$fonts_install_directory  2>> log.log
