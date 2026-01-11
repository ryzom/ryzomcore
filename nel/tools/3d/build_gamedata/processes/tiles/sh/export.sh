#!/bin/bash
rm log.log 2> /dev/null

# Export the tile textures

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the tile directories
tile_source_directories=`cat ../../cfg/directories.cfg | grep "tile_source_directories" | sed -e 's/tile_source_directories//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Export tiles >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export tiles 
echo ------- 
date >> log.log
date

# For each directoy
for i in $tile_source_directories ; do
	list_textures=`find $database_directory/$i -type f -name '*.[tT][gG][aA]'`

	# For each textures
	for j in $list_textures ; do
		cp -u -p $j maps_tga/ 2>> log.log
	done

	# Idle
	../../idle.bat
done
