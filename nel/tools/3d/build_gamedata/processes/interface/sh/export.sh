#!/bin/bash
rm log.log 2> /dev/null

# Bin
tga_2_dds='tga2dds.exe'

# *** Export interface tile (.tga)

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the interface fullscreen directories
interface_fullscreen_directories=`cat ../../cfg/directories.cfg | grep "interface_fullscreen_directories" | sed -e 's/interface_fullscreen_directories//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the interface 3d directories
interface_3d_directories=`cat ../../cfg/directories.cfg | grep "interface_3d_directories" | sed -e 's/interface_3d_directories//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Export interface >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export interface 
echo ------- 
date >> log.log
date

# For each interface fullscreen directory compress independently all in dds
rm tga_tmp/*.[tT][gG][aA]
for i in $interface_fullscreen_directories; do
	# Copy
	cp -u -p $database_directory/$i/*.[tT][gG][aA] tga_tmp 2>> log.log

	# Idle
	../../idle.bat
done

for i in tga_tmp/*.[tT][gG][aA] ; do

	# Destination file
	dest=`echo $i | sed -e 's/tga_tmp/tga/g'`
	dest=`echo $dest | sed -e 's/\.[tT][gG][aA]/.dds/g'`

	if ( ! test -e $dest ) || ( test $i -nt $dest )
	then
		# Convert
		$tga_2_dds $i -o $dest -a 5 2>> log.log
	fi

	# Idle
	../../idle.bat
done


# For each interface 3d directory
for i in $interface_3d_directories; do
	# Copy
	cp -u -p $database_directory/$i/* 3d 2>> log.log

	# Idle
	../../idle.bat
done
