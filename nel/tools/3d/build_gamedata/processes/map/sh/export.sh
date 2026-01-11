#!/bin/bash
rm log.log 2> /dev/null

# *** Export maps files (.tga / *.[dD][dD][sS]) from the database

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the maps directories
map_source_directories=`cat ../../cfg/directories.cfg | grep "map_source_directory" | sed -e 's/map_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`
map_uncompressed_source_directories=`cat ../../cfg/directories.cfg | grep "map_uncompressed_source_directory" | sed -e 's/map_uncompressed_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`


# Log error
echo ------- > log.log
echo --- Export map >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export map 
echo ------- 
date >> log.log
date

# For each directoy

for i in $map_source_directories; do
	for j in $database_directory/$i/*.[tT][gG][aA]; do
		if ( test -f $j )
		then
			# Get the dds version
			dds=`echo $j | sed -e 's&.tga&.dds&g'`

			# Copy the dds and the tga
			cp -u -p $j tga 2>> log.log
			if ( test -f $dds )
			then
				cp -u -p $dds tga 2>> log.log
			fi
		fi
	done
	# Idle
	../../idle.bat
done

for i in $map_uncompressed_source_directories; do
	for j in $database_directory/$i/*.[tT][gG][aA]; do
		if ( test -f $j )
		then
			# Copy the dds and the tga
			cp -u -p $j tga_uncompressed 2>> log.log
		fi
	done
	# Idle
	../../idle.bat
done


