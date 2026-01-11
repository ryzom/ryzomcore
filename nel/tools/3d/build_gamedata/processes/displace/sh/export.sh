#!/bin/bash
rm log.log 2> /dev/null

# *** Export displace tile (.tga)

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the displace directories
displace_source_directories=`cat ../../cfg/directories.cfg | grep "displace_source_directories" | sed -e 's/displace_source_directories//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Export displace >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export displace 
echo ------- 
date >> log.log
date

# For each displace directory
for i in $displace_source_directories ; do
	# Copy
	cp -u -p $database_directory/$i/*.[tT][gG][aA] tga 2>> log.log

	# Idle
	../../idle.bat
done
