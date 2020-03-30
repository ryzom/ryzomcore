#!/bin/bash
rm log.log 2> /dev/null

# Build zone

zone_lighter='zone_lighter.exe'
zone_ig_lighter='zone_ig_lighter.exe'
exec_timeout='exec_timeout.exe'

# Get the timeout
light_timeout=`cat ../../cfg/config.cfg | grep "zone_build_light_timeout" | sed -e 's/zone_build_light_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`
ig_light_timeout=`cat ../../cfg/config.cfg | grep "zone_build_ig_light_timeout" | sed -e 's/zone_build_ig_light_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`

# **** Light

# Log error
echo ------- >> log.log
echo --- Zone lighting >> log.log
echo ------- >> log.log
echo ------- 
echo --- Zone lighting
echo ------- 
date >> log.log
date

#append the level design directory at the end of the config file
ld_dir=`cat ../../cfg/site.cfg | grep "level_design_directory" | sed -e 's/level_design_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`
ld_world_dir=`cat ../../cfg/site.cfg | grep "level_design_world_directory" | sed -e 's/level_design_world_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`
ld_dfn_dir=`cat ../../cfg/site.cfg | grep "level_design_dfn_directory" | sed -e 's/level_design_dfn_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`
continent_file_name=`cat ../../cfg/config.cfg | grep "continent_file" | sed -e 's/continent_file//g' | sed -e 's/ //g' | sed -e 's/=//g'`
cp ../../cfg/properties.cfg zone_lighter_properties.cfg
echo "level_design_directory = \"$ld_dir\";" >> zone_lighter_properties.cfg
echo "level_design_world_directory = \"$ld_world_dir\";" >> zone_lighter_properties.cfg
echo "level_design_dfn_directory = \"$ld_dfn_dir\";" >> zone_lighter_properties.cfg
echo "continent_name = \"$continent_file_name\";" >> zone_lighter_properties.cfg

# List the zones to light
list_zone_welded=`ls -1 ../zone/zone_welded/*.[zZ][oO][nN][eE][wW]`

# create a bkup directory
mkdir bkup_tile_water

# Light zones
for i in $list_zone_welded ; do
	dest=`echo $i | sed -e 's&../zone/zone_welded&zone_lighted&g' | sed -e 's/.zonew/.zonel/g'`
	depend=`echo $i | sed -e 's&../zone/zone_welded&../zone/zone_depend&g' | sed -e 's/.zonew/.depend/g'`
	if ( test -e $dest )
	then
		echo PATCH $dest
		echo PATCH $dest >> log.log 	        
		# patch, and bkup if necessary
		$exec_timeout $light_timeout $zone_lighter $i $dest zone_lighter_properties.cfg $depend -waterpatch bkup_tile_water
		echo 
		echo 
	else
		echo SKIP $dest cause not found
		echo SKIP $dest cause not found >> log.log 	        
	fi

	# Idle
	../../idle.bat
done

