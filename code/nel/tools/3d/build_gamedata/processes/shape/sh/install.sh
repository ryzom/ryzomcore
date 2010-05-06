#!/bin/bash
rm log.log 2> /dev/null

# Install shapes in the client data

# Get the shape install directory
shape_install_directory=`cat ../../cfg/directories.cfg | grep "shape_install_directory" | sed -e 's/shape_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the lightmaps install directory
lightmap_install_directory=`cat ../../cfg/directories.cfg | grep "lightmap_install_directory" | sed -e 's/lightmap_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../../cfg/site.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Install Shape >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install Shape 
echo ------- 
date >> log.log
date


cp -u -p -R shape/. $client_directory/$shape_install_directory  2>> log.log
cp -u -p -R shape_with_coarse_mesh_builded/. $client_directory/$shape_install_directory  2>> log.log

if test "$lightmap_install_directory"; then
	mkdir $client_directory/$lightmap_install_directory 2>> log.log 2> /dev/null
	cp -u -p -R lightmap_16_bits/. $client_directory/$lightmap_install_directory  2>> log.log
fi

cp -u -p -R anim/. $client_directory/$shape_install_directory  2>> log.log

ls anim | grep ".anim" >> $client_directory/auto_animations_list.txt
