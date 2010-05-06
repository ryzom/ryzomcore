#!/bin/bash
rm log.log 2> /dev/null

# *********************************************
# *********************************************
# *** Export shape files (.shape) from Max
# *********************************************
# *********************************************

exec_timeout='exec_timeout.exe'

# Get the timeout
timeout=`cat ../../cfg/config.cfg | grep "shape_export_timeout" | sed -e 's/shape_export_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the build gamedata directory
build_gamedata_directory=`cat ../../cfg/site.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the shape directories
shape_source_directories=`cat ../../cfg/directories.cfg | grep "shape_source_directory" | sed -e 's/shape_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the quality option to choose the goor properties.cfg file
quality_flag=`cat ../../cfg/site.cfg | grep "build_quality" | grep "1"`

# Maxdir
max_directory=`echo $MAX_DIR | sed -e 's&\\\&/&g'`

# Get the options
if ( test "$quality_flag" )
then
	# We are in BEST mode
	seoel=`cat ../../cfg/config.cfg | grep "shape_export_opt_export_lighting" | sed -e 's/shape_export_opt_export_lighting//' | sed -e 's/ //g' | sed -e 's/=//g'`
	seos=`cat ../../cfg/config.cfg | grep "shape_export_opt_shadow" | sed -e 's/shape_export_opt_shadow//' | sed -e 's/ //g' | sed -e 's/=//g'`
	seoll=`cat ../../cfg/config.cfg | grep "shape_export_opt_lighting_limit" | sed -e 's/shape_export_opt_lighting_limit//' | sed -e 's/ //g' | sed -e 's/=//g'`
	seols=`cat ../../cfg/config.cfg | grep "shape_export_opt_lumel_size" | sed -e 's/shape_export_opt_lumel_size//' | sed -e 's/ //g' | sed -e 's/=//g'`
	seoo=`cat ../../cfg/config.cfg | grep "shape_export_opt_oversampling" | sed -e 's/shape_export_opt_oversampling//' | sed -e 's/ //g' | sed -e 's/=//g'`
else
	# We are in DRAFT mode
	seoel='false'
	seos='false'
	seoll='0'
	seols='0.25'
	seoo='1'
fi
	seolog=`cat ../../cfg/config.cfg | grep "shape_export_opt_lightmap_log" | sed -e 's/shape_export_opt_lightmap_log//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Export shape >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export shape 
echo ------- 
date >> log.log
date

# For each directoy

for i in $shape_source_directories ; do
	# Copy the script
	cat maxscript/shape_export.ms | sed -e "s&output_logfile&$build_gamedata_directory/processes/shape/log.log&g" | sed -e "s&shape_source_directory&$database_directory/$i&g" | sed -e "s&output_directory_tag&$build_gamedata_directory/processes/shape/tag&g" | sed -e "s&output_directory_without_coarse_mesh&$build_gamedata_directory/processes/shape/shape_not_optimized&g" | sed -e "s&output_directory_with_coarse_mesh&$build_gamedata_directory/processes/shape/shape_with_coarse_mesh&g" | sed -e "s&shape_export_opt_export_lighting&$seoel&g" | sed -e "s&shape_export_opt_shadow&$seos&g" | sed -e "s&shape_export_opt_lighting_limit&$seoll&g" | sed -e "s&shape_export_opt_lumel_size&$seols&g" | sed -e "s&shape_export_opt_oversampling&$seoo&g"| sed -e "s&shape_export_opt_lightmap_log&$seolog&g" | sed -e "s&shape_lightmap_path&$build_gamedata_directory/processes/shape/lightmap_not_optimized&g"  | sed -e "s&output_directory_anim&$build_gamedata_directory/processes/shape/anim&g" > $max_directory/scripts/shape_export.ms

	# Start max
	echo Try 1 >> log.log
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript shape_export.ms -q -mi -vn

	# Idle
	../../idle.bat

	echo Try 2 >> log.log
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript shape_export.ms -q -mi -vn

	# Idle
	../../idle.bat

	echo Try 3 >> log.log
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript shape_export.ms -q -mi -vn

	# Idle
	../../idle.bat
done


# *********************************************
# *********************************************
# *** Export character lod shape files (.clod) from Max
# *********************************************
# *********************************************

# Get the clod directories
clod_source_directories=`cat ../../cfg/directories.cfg | grep "clod_source_directory" | sed -e 's/clod_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- >> log.log
echo --- Export clod >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export clod
echo ------- 
date >> log.log
date

# For each directoy

for i in $clod_source_directories ; do
	# Copy the script. TAKE IT FROM clodbank process. But write it here.
	cat ../clodbank/maxscript/clod_export.ms | sed -e "s&shape_source_directory&$database_directory/$i&g" | sed -e "s&output_directory_clod&$build_gamedata_directory/processes/shape/clod&g" | sed -e "s&output_directory_tag&$build_gamedata_directory/processes/shape/tag&g" > $max_directory/scripts/clod_export.ms

	# Start max
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript clod_export.ms -q -mi -vn

	# Concat log.log files
	echo Try 1 >> log.log
	cat $max_directory/log.log >> log.log

	# Idle
	../../idle.bat

	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript clod_export.ms -q -mi -vn

	# Concat log.log files
	echo Try 2 >> log.log
	cat $max_directory/log.log >> log.log

	# Idle
	../../idle.bat

	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript clod_export.ms -q -mi -vn

	# Concat log.log files
	echo Try 3 >> log.log
	cat $max_directory/log.log >> log.log

	# Idle
	../../idle.bat
done


