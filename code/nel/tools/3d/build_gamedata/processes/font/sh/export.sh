#!/bin/bash
rm log.log 2> /dev/null

# *** Export fonts

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the ps directories
font_source_directories=`cat ../../cfg/directories.cfg | grep "font_source_directories" | sed -e 's/font_source_directories//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Export fonts >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export fonts
echo ------- 
date >> log.log
date

# For each font directory
for i in $font_source_directories ; do
	# Copy
	cp -u -p $database_directory/$i/*.[tT][tT][fF] fonts 2>> log.log
	cp -u -p $database_directory/$i/*.[aA][fF][mM] fonts 2>> log.log
	cp -u -p $database_directory/$i/*.[pP][fF][bB] fonts 2>> log.log
	cp -u -p $database_directory/$i/*.[pP][fF][mM] fonts 2>> log.log

	# Idle
	../../idle.bat
done
