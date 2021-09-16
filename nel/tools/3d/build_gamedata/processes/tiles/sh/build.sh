#!/bin/bash
rm log.log 2> /dev/null

tga_2_dds='tga2dds.exe'
exec_timeout='exec_timeout.exe'

# Get the timeout
timeout=`cat ../../cfg/config.cfg | grep "maps_build_timeout" | sed -e 's/maps_build_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Build the tile textures

# Log error
echo ------- > log.log
echo --- Build tiles >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build tiles 
echo ------- 
date >> log.log
date

# For each texture
for i in maps_tga/*.[tT][gG][aA] ; do
	if ( test -e $i )
	then
		dest=`echo $i | sed -e 's/maps_tga/maps_final/g' | sed -e 's/.tga/.dds/g'`
		if ( ! test -e $dest ) || ( test $i -nt $dest )
		then
			$exec_timeout $timeout $tga_2_dds $i -o $dest -a 5 -m
			if ( test -e $dest )
			then
				echo OK $dest >> log.log
			else
				echo ERROR building $dest >> log.log
			fi
		else
			echo SKIPPED $dest >> log.log
		fi
	fi

	# Idle
	../../idle.bat
done
