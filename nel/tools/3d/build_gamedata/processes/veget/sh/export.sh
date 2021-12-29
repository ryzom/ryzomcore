#!/bin/bash
rm log.log 2> /dev/null

# *** Export veget files (.veget) from Max

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the build gamedata directory
build_gamedata_directory=`cat ../../cfg/site.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the veget directories
veget_source_directories=`cat ../../cfg/directories.cfg | grep "veget_source_directory" | sed -e 's/veget_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Maxdir
max_directory=`echo $MAX_DIR | sed -e 's&\\\&/&g'`

# Log error
echo ------- > log.log
echo --- Export veget >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export veget 
echo ------- 
date >> log.log
date

# For each directoy

for i in $veget_source_directories ; do
	# Copy the script
	cat maxscript/veget_export.ms | sed -e "s&output_logfile&$build_gamedata_directory/processes/veget/log.log&g" | sed -e "s&veget_source_directory&$database_directory/$i&g" | sed -e "s&output_directory_veget&$build_gamedata_directory/processes/veget/veget&g" | sed -e "s&output_directory_tag&$build_gamedata_directory/processes/veget/tag&g" > $max_directory/scripts/veget_export.ms

	# Start max
	echo Try 1 >> log.log
	$max_directory/3dsmax.exe -U MAXScript veget_export.ms -q -mi -vn

	# Idle
	../../idle.bat

	echo Try 2 >> log.log
	$max_directory/3dsmax.exe -U MAXScript veget_export.ms -q -mi -vn

	# Idle
	../../idle.bat

	echo Try 3 >> log.log
	$max_directory/3dsmax.exe -U MAXScript veget_export.ms -q -mi -vn

	# Idle
	../../idle.bat
done
