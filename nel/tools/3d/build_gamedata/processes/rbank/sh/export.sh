#!/bin/bash
rm log.log 2> /dev/null

# *** Export cmb files (.cmb) from Max

exec_timeout='exec_timeout.exe'

# Get the timeout
timeout=`cat ../../cfg/config.cfg | grep "cmb_export_timeout" | sed -e 's/cmb_export_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the build gamedata directory
build_gamedata_directory=`cat ../../cfg/site.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the collision directories
collision_source_directories=`cat ../../cfg/directories.cfg | grep "cmb_source_directory" | sed -e 's/cmb_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Maxdir
max_directory=`echo $MAX_DIR | sed -e 's&\\\&/&g'`

# Log error
echo ------- > log.log
echo --- Export cmb for rbank >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export cmb for rbank
echo ------- 
date >> log.log
date

# For each directoy

for i in $collision_source_directories ; do
	# Copy the script
	cat maxscript/rbank_export.ms | sed -e "s&output_logfile&$build_gamedata_directory/processes/rbank/log.log&g" | sed -e "s&collision_source_directory&$database_directory/$i&g" | sed -e "s&output_directory_rbank&$build_gamedata_directory/processes/rbank/cmb&g" | sed -e "s&output_directory_tag&$build_gamedata_directory/processes/rbank/tag&g" > $max_directory/scripts/rbank_export.ms

	# Start max
	echo Try 1 >> log.log
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript rbank_export.ms -q -mi -vn

	# Idle
	../../idle.bat

	echo Try 2 >> log.log
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript rbank_export.ms -q -mi -vn

	# Idle
	../../idle.bat

	echo Try 3 >> log.log
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript rbank_export.ms -q -mi -vn

	# Idle
	../../idle.bat
done
