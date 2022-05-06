#!/bin/bash
rm log.log 2> /dev/null

# *** Export bank file (.bank) from Max

# Some exe
build_smallbank='build_smallbank.exe'
exec_timeout='exec_timeout.exe'

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the swt directories
bank_source_directory=`cat ../../cfg/directories.cfg | grep "bank_source_directory" | sed -e 's/bank_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the timeout
timeout=`cat ../../cfg/config.cfg | grep "smallbank_build_timeout" | sed -e 's/smallbank_build_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the tiles root directories
tile_root_source_directory=`cat ../../cfg/directories.cfg | grep "tile_root_source_directory" | sed -e 's/tile_root_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Export bank >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export bank 
echo ------- 
date >> log.log
date

# Copy the bank
cp -u -p $database_directory/$bank_source_directory/*.[bB][aA][nN][kK] bank 2>> log.log

# Build the small bank

# Log error
echo ------- > log.log
echo --- Build bank >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build bank 
echo ------- 
date >> log.log
date

# list all the bank
bank_list=`ls -1 bank/*.[bB][aA][nN][kK]`

# For each bank
for i in $bank_list ; do
	# Destination the name
	dest=`echo $i | sed -e 's&bank&smallbank&g'`

	# Make the dependencies
	if ( ! test -e $dest ) || ( test $i -nt $dest ) 
	then
		$exec_timeout $timeout $build_smallbank $i $dest $database_directory/$tile_root_source_directory/
		if ( test -e $dest )
		then
			echo OK $dest >> log.log
		else
			echo ERROR building $dest >> log.log
		fi
	else
		echo SKIPPED $dest >> log.log
	fi

	# Idle
	../../idle.bat
done

