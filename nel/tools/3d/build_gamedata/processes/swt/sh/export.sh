#!/bin/bash
rm log.log 2> /dev/null

# *** Export skeleton weight files (.swt) from Max

exec_timeout='exec_timeout.exe'

# Get the timeout
timeout=`cat ../../cfg/config.cfg | grep "swt_export_timeout" | sed -e 's/swt_export_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the build gamedata directory
build_gamedata_directory=`cat ../../cfg/site.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the swt directories
swt_source_directories=`cat ../../cfg/directories.cfg | grep "swt_source_directory" | sed -e 's/swt_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Maxdir
max_directory=`echo $MAX_DIR | sed -e 's&\\\&/&g'`

# Log error
echo ------- > log.log
echo --- Export skeleton weight >> log.log
echo ------- >> log.log
echo -------
echo --- Export skeleton weight
echo -------
date >> log.log
date

# For each directoy

for i in $swt_source_directories ; do
	# Copy the script
	cat maxscript/swt_export.ms | sed -e "s&output_logfile&$build_gamedata_directory/processes/swt/log.log&g" | sed -e "s&swt_source_directory&$database_directory/$i&g" | sed -e "s&output_directory&$build_gamedata_directory/processes/swt/swt&g" > $max_directory/scripts/swt_export.ms

	# Start max
	echo Try 1 >> log.log
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript swt_export.ms -q -mi -vn

	# Idle
	../../idle.bat

	echo Try 2 >> log.log
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript swt_export.ms -q -mi -vn

	# Idle
	../../idle.bat

	echo Try 3 >> log.log
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript swt_export.ms -q -mi -vn

	# Idle
	../../idle.bat
done
