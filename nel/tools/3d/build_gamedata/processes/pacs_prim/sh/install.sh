#!/bin/bash
rm log.log 2> /dev/null

# Install the zonels in the client data

# Get the zone install directory
pacs_primitive_install_directory=`cat ../../cfg/directories.cfg | grep "pacs_primitive_install_directory" | sed -e 's/pacs_primitive_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../../cfg/site.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Test if there is a need for the prim pacs directory
want_landscape_col_prim_pacs_list=`cat ../../cfg/config.cfg | grep "want_landscape_col_prim_pacs_list"`

# Log error
echo ------- > log.log
echo --- Install zone >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install zone 
echo ------- 
date >> log.log
date

cp -u -p -R pacs_prim/. $client_directory/$pacs_primitive_install_directory  2>> log.log

if test "$want_landscape_col_prim_pacs_list" ; then
   ls pacs_prim | grep ".pacs_prim" >> $client_directory/landscape_col_prim_pacs_list.txt
fi

