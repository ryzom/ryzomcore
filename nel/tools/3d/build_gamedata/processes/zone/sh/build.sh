#!/bin/bash
rm log.log 2> /dev/null

# Build zone

zone_dependencies='zone_dependencies.exe'
zone_welder='zone_welder.exe'
exec_timeout='exec_timeout.exe'

# Get the timeout
depend_timeout=`cat ../../cfg/config.cfg | grep "zone_build_depend_timeout" | sed -e 's/zone_build_depend_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`
weld_timeout=`cat ../../cfg/config.cfg | grep "zone_build_weld_timeout" | sed -e 's/zone_build_weld_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the quality option to choose the goor properties.cfg file
quality_flag=`cat ../../cfg/site.cfg | grep "build_quality" | grep "1"`

# **** Build dependencies

if ( test "$quality_flag" )
then
	# We are in BEST mode

	# Log error
	echo ------- >> log.log
	echo --- Build zone : dependencies >> log.log
	echo ------- >> log.log
	echo ------- 
	echo --- Build zone : dependencies 
	echo ------- 
	date >> log.log
	date

	cp ../../cfg/properties.cfg zone_depencies_properties.cfg
	#append the level design directory at the end of the config file
	ld_dir=`cat ../../cfg/site.cfg | grep "level_design_directory" | sed -e 's/level_design_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`
	ld_world_dir=`cat ../../cfg/site.cfg | grep "level_design_world_directory" | sed -e 's/level_design_world_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`
	ld_dfn_dir=`cat ../../cfg/site.cfg | grep "level_design_dfn_directory" | sed -e 's/level_design_dfn_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`
	continent_file_name=`cat ../../cfg/config.cfg | grep "continent_file" | sed -e 's/continent_file//g' | sed -e 's/ //g' | sed -e 's/=//g'`	
	echo "level_design_directory = \"$ld_dir\";" >> zone_depencies_properties.cfg
	echo "level_design_world_directory = \"$ld_world_dir\";" >> zone_depencies_properties.cfg
	echo "level_design_dfn_directory = \"$ld_dfn_dir\";" >> zone_depencies_properties.cfg
	echo "continent_name = \"$continent_file_name\";" >> zone_depencies_properties.cfg


	# list all the dependencies regions
	zone_regions=`cat ../../cfg/config.cfg | grep "zone_region" | sed -e 's/zone_region//' | sed -e 's/ //g' | sed -e 's/=//g'`

	# For each dependencies region
	for i in $zone_regions ; do
		# Extract the name
		arg=`echo zone_exported/$zone_regions | sed -e 's&,&.zone zone_exported/&g'`		
		# Make the dependencies
		$exec_timeout $depend_timeout $zone_dependencies zone_depencies_properties.cfg $arg.zone zone_depend/doomy.depend

		# Idle
		../../idle.bat
	done
fi	

# **** Weld

# Log error
echo ------- >> log.log
echo --- Build zone : weld >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build zone : weld 
echo ------- 
date >> log.log
date

# List the zones to weld
list_zone=`ls -1 zone_exported/*.[zZ][oO][nN][eE]`

# Build a zones list to weld
echo -- Build a list of file to weld
rm zone_to_weld.txt 2> /dev/null
for i in $list_zone ; do
  dest=`echo $i | sed -e 's/zone_exported/zone_welded/g' | sed -e 's/.zone/.zonew/g'`
  if ( ! test -e $dest ) || ( test $i -nt $dest )
  then
	echo $i >> zone_to_weld.txt
	rm $dest
  fi

	# Idle
	../../idle.bat
done

# Weld the zone
if (test -f zone_to_weld.txt) 
then
	list_zone=`cat zone_to_weld.txt`
	for i in $list_zone ; do
		echo -- Weld $i
		echo -- Weld $i >> log.log
	    $exec_timeout $weld_timeout $zone_welder $i $dest
		echo 

		# Idle
		../../idle.bat
	done
fi

# Log error
echo ------- >> log.log
echo --- Build zone : weld zones without heightmap >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build zone : weld zones without heightmap 
echo ------- 
date >> log.log
date

# List the zones to weld
list_zone=`ls -1 zone_exported/*.[zZ][oO][nN][eE][nN][hH]`

# Build a zones list to weld
echo -- Build a list of file to weld
rm zone_to_weld.txt 2> /dev/null
for i in $list_zone ; do
  dest=`echo $i | sed -e 's/zone_exported/zone_welded/g' | sed -e 's/.zonenh/.zonenhw/g'`
  if ( ! test -e $dest ) || ( test $i -nt $dest )
  then
	echo $i >> zone_to_weld.txt
	rm $dest
  fi

	# Idle
	../../idle.bat
done

# Weld the zone
if (test -f zone_to_weld.txt) 
then
	list_zone=`cat zone_to_weld.txt`
	for i in $list_zone ; do
		echo -- Weld $i
		echo -- Weld $i >> log.log
	    $exec_timeout $weld_timeout $zone_welder $i $dest
		echo 

		# Idle
		../../idle.bat
	done
fi

# Build a zones list to weld
rm zone_to_weld.txt 2> /dev/null
