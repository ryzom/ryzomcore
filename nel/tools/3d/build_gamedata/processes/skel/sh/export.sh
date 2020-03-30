#!/bin/bash
rm log.log 2> /dev/null

# *** Export skeleton files (.skel) from Max

exec_timeout='exec_timeout.exe'

# Get the timeout
timeout=`cat ../../cfg/config.cfg | grep "skel_export_timeout" | sed -e 's/skel_export_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the build gamedata directory
build_gamedata_directory=`cat ../../cfg/site.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the skel directories
skel_source_directories=`cat ../../cfg/directories.cfg | grep "skel_source_directory" | sed -e 's/skel_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Maxdir
max_directory=`echo $MAX_DIR | sed -e 's&\\\&/&g'`

# Log error
echo ------- > log.log
echo --- Export skeleton from MAX>> log.log
echo ------- >> log.log
echo ------- 
echo --- Export skeleton from MAX
echo ------- 
date >> log.log
date

# For each directoy

for i in $skel_source_directories ; do
	# Copy the script
	cat maxscript/skel_export.ms | sed -e "s&output_logfile&$build_gamedata_directory/processes/skel/log.log&g" | sed -e "s&skel_source_directory&$database_directory/$i&g" | sed -e "s&output_directory&$build_gamedata_directory/processes/skel/skel&g" > $max_directory/scripts/skel_export.ms

	# Start max
	echo Try 1 >> log.log
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript skel_export.ms -q -mi -vn

	# Idle
	../../idle.bat

	echo Try 2 >> log.log
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript skel_export.ms -q -mi -vn

	# Idle
	../../idle.bat

	echo Try 3 >> log.log
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript skel_export.ms -q -mi -vn

	# Idle
	../../idle.bat
done


# *** Export skeleton files (.skel) directly from .skel version

# Log error
echo ------- >> log.log
echo --- Copy skeleton from .skel>> log.log
echo ------- >> log.log
echo ------- 
echo --- Copy skeleton from .skel
echo ------- 
date >> log.log
date

# For each directoy

for i in $skel_source_directories ; do
	# copy
	cp -u -p $database_directory/$i/*.[sS][kK][eE][lL]  skel  2>> log.log

	# Idle
	../../idle.bat
done
