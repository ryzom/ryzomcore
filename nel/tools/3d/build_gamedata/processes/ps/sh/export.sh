#!/bin/bash
rm log.log 2> /dev/null

# *** Export particle system file (.ps)

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the ps directories
ps_source_directories=`cat ../../cfg/directories.cfg | grep "ps_source_directory" | sed -e 's/ps_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Export ps >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export ps 
echo ------- 
date >> log.log
date

# For each ps directory
for i in $ps_source_directories ; do
	# Copy
	cp -u -p $database_directory/$i/*.[pP][sS] ps 2>> log.log
	cp -u -p $database_directory/$i/*.[sS][hH][aA][pP][eE] ps 2>> log.log
	cp -u -p $database_directory/$i/*.[pP][rR][iI][mM][iI][tT][iI][vV][eE] ps 2>> log.log

	# Idle
	../../idle.bat
done
