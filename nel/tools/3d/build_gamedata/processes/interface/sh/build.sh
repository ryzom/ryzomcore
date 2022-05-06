#!/bin/bash
rm log.log 2> /dev/null


# *** Build interface

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the interface directories
interface_source_directories=`cat ../../cfg/directories.cfg | grep "interface_source_directories" | sed -e 's/interface_source_directories//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the interface directories to compress in one DXTC only
interface_source_dxtc_directories=`cat ../../cfg/directories.cfg | grep "interface_source_dxtc_directories" | sed -e 's/interface_source_dxtc_directories//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Build interface >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build interface 
echo ------- 
date >> log.log
date

# For each interface directory
for i in $interface_source_directories ; do
	# Copy
	niouname=`echo $i | sed -e 's&/&_&g'`
	rm tga_tmp/*.[tT][gG][aA]
	cp -u -p $database_directory/$i/*.[tT][gG][aA] tga_tmp 2>> log.log
	build_interface.exe tga/texture_$niouname.tga tga_tmp

	# Idle
	../../idle.bat
done


# For each interface directory to compress in one DXTC
rm tga_tmp/*.[tT][gG][aA]
for i in $interface_source_dxtc_directories ; do
	# Copy
	cp -u -p $database_directory/$i/*.[tT][gG][aA] tga_tmp 2>> log.log
done

# build all files in tga_tmp into one tga. Let the OpenGL compress it at runTime (better result for some important cases)
build_interface.exe tga/texture_interfaces_dxtc.tga tga_tmp
# DO NOT COMPRESS ../../bin/tga2dds tga_tmp/texture_interfaces_dxtc.tga -o tga/texture_interfaces_dxtc.dds -a 5


# Compress animation. Important to avoid 'memory fragmentation'
mv -u 3d/*.[aA][nN][iI][mM] anim_tmp 2>> log.log
anim_builder.exe anim_tmp 3d ../../cfg/properties.cfg



# Idle
../../idle.bat


