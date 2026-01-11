#!/bin/bash
rm log.log 2> /dev/null

# *** Export zone files (.zone) from Max

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the water maps directories
water_map_directories=`cat ../../cfg/directories.cfg | grep "water_map_directory" | sed -e 's/water_map_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Export water shape >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export water shape 
echo ------- 
date >> log.log
date

#copy each water map before lightmapping
for i in $water_map_directories ; do
	cp -u -p $database_directory/$i/*.[tT][gG][aA] water_shapes_lighted 2>> log.log

	# Idle
	../../idle.bat
done
