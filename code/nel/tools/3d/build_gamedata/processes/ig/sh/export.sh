#!/bin/bash
rm log.log 2> /dev/null

# *** Export ig files (.ig) from Max

exec_timeout='exec_timeout.exe'

# Get the timeout
timeout=`cat ../../cfg/config.cfg | grep "ig_export_timeout" | sed -e 's/ig_export_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the build gamedata directory
build_gamedata_directory=`cat ../../cfg/site.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the ig directories
ig_land_source_directories=`cat ../../cfg/directories.cfg | grep "ig_land_source_directory" | sed -e 's/ig_land_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the ig directories
ig_other_source_directories=`cat ../../cfg/directories.cfg | grep "ig_other_source_directory" | sed -e 's/ig_other_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Maxdir
max_directory=`echo $MAX_DIR | sed -e 's&\\\&/&g'`

# Log error
echo ------- > log.log
echo --- Export ig >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export ig 
echo -------
date >> log.log
date

# For each directoy

# List landscape ig


for i in $ig_land_source_directories ; do
	# Copy the script
	cat maxscript/ig_export.ms | sed -e "s&output_logfile&$build_gamedata_directory/processes/ig/log.log&g" | sed -e "s&ig_source_directory&$database_directory/$i&g" | sed -e "s&output_directory_tag&$build_gamedata_directory/processes/ig/tag&g" | sed -e "s&output_directory_ig&$build_gamedata_directory/processes/ig/ig_land_max&g" > $max_directory/scripts/ig_export.ms

	# Start max
	echo Try 1 >> log.log
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript ig_export.ms -q -mi -vn

	# Idle
	../../idle.bat

	echo Try 2 >> log.log
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript ig_export.ms -q -mi -vn

	# Idle
	../../idle.bat

	echo Try 3 >> log.log
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript ig_export.ms -q -mi -vn

	# Idle
	../../idle.bat
done




for i in $ig_other_source_directories ; do
	# Copy the script
	cat maxscript/ig_export.ms | sed -e "s&output_logfile&$build_gamedata_directory/processes/ig/log.log&g" | sed -e "s&ig_source_directory&$database_directory/$i&g" | sed -e "s&output_directory_tag&$build_gamedata_directory/processes/ig/tag&g" | sed -e "s&output_directory_ig&$build_gamedata_directory/processes/ig/ig_other&g" > $max_directory/scripts/ig_export.ms

	# Start max
	echo Try 1 >> log.log
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript ig_export.ms -q -mi -vn

	# Idle
	../../idle.bat

	echo Try 2 >> log.log
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript ig_export.ms -q -mi -vn

	# Idle
	../../idle.bat

	echo Try 3 >> log.log
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript ig_export.ms -q -mi -vn

	# Idle
	../../idle.bat
done
