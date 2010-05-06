#!/bin/bash
rm log.log 2> /dev/null

# *** Export zone files (.zone) from Max

exec_timeout='exec_timeout.exe'

# Get the timeout
timeout=`cat ../../cfg/config.cfg | grep "zone_export_timeout" | sed -e 's/zone_export_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the build gamedata directory
build_gamedata_directory=`cat ../../cfg/site.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the zone directories
zone_source_directories=`cat ../../cfg/directories.cfg | grep "zone_source_directory" | sed -e 's/zone_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the ligo value
ligo_flag=`cat ../../cfg/config.cfg | grep "process_to_complete" | grep "ligo"`

# Maxdir
max_directory=`echo $MAX_DIR | sed -e 's&\\\&/&g'`

if ( test "$ligo_flag" )
then
	echo [Ligo] ON
	echo [Ligo] ON >> log.log
else
	echo [Ligo] OFF
	echo [Ligo] OFF >> log.log
fi


# Log error
echo ------- > log.log
echo --- Export zone >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export zone 
echo ------- 
date >> log.log
date

# Try to export from Max zone if any

for i in $zone_source_directories ; do
	# Copy the script
	cat maxscript/zone_export.ms | sed -e "s&output_logfile&$build_gamedata_directory/processes/zone/log.log&g" | sed -e "s&zone_source_directory&$database_directory/$i&g" | sed -e "s&output_directory&$build_gamedata_directory/processes/zone/zone_exported&g" > $max_directory/scripts/zone_export.ms

	# Start max
	echo Try 1 >> log.log
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript zone_export.ms -q -mi -vn

	echo Try 2 >> log.log
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript zone_export.ms -q -mi -vn

	echo Try 3 >> log.log
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript zone_export.ms -q -mi -vn

	# Idle
	../../idle.bat
done

# ****************************
# Try to copy ligo zone if any
# ****************************

dir_current=`pwd`
cd ../ligo/output
list_zone=`ls -1 *.[zZ][oO][nN][eE]`
for filename in $list_zone ; do
	echo "Checking $filename for update"
	if test -e ../../zone/zone_exported/$filename ; then
		must_update=`diff --binary -q $filename ../../zone/zone_exported/$filename` ;
	else
		must_update=YES ;
	fi
	
	if test -n "$must_update" ; then
		echo "   Updating"
		cp -u -p $filename ../../zone/zone_exported/$filename ;
	fi

	# Idle
	../../../idle.bat
done
cd $dir_current

# delete files only present in the zone_exported directory

if ( test "$ligo_flag" )
then
	cd ./zone_exported
	list_zone=`ls -1 *.[zZ][oO][nN][eE]`
	for filename in $list_zone ; do
		if test -e ../../ligo/output/$filename ; then
			must_update=NO ;
		else
			echo "Removing $filename"
			rm $filename ;
		fi

		# Idle
		../../../idle.bat
	done
	cd ..
fi
