#!/bin/bash
rm log.log 2> /dev/null

# Build the farbank

build_farbank='build_far_bank.exe'
exec_timeout='exec_timeout.exe'

# Get the timeout
timeout=`cat ../../cfg/config.cfg | grep "farbank_build_timeout" | sed -e 's/farbank_build_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the tiles root directories
tile_root_source_directory=`cat ../../cfg/directories.cfg | grep "tile_root_source_directory" | sed -e 's/tile_root_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the extension list
multiple_tiles_postfix=`cat ../../cfg/config.cfg | grep "multiple_tiles_postfix" | sed -e 's/multiple_tiles_postfix//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Build farbank >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build farbank 
echo ------- 
date >> log.log
date

# list all the bank
bank_list=`ls -1 ../smallbank/smallbank/*.[sS][mM][aA][lL][lL][bB][aA][nN][kK]`

# For each bank
for i in $bank_list ; do

	if ( test "$multiple_tiles_postfix" ) then

		for j in $multiple_tiles_postfix ; do

			# Destination the name
			dest=`echo $i | sed -e "s&\.smallbank&\$j.farbank&g" | sed -e 's&../smallbank/smallbank&farbank&g'`
			echo $i
			echo $dest

			# Make the dependencies
			if ( ! test -e $dest ) || ( test $i -nt $dest ) 
			then
				$exec_timeout $timeout $build_farbank $i $dest -d$database_directory/$tile_root_source_directory$j/ -p$j
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

	else

		# Destination the name
		dest=`echo $i | sed -e 's&\.smallbank&\.farbank&g' | sed -e 's&../smallbank/smallbank&farbank&g'`
		echo $i
		echo $dest

		# Make the dependencies
		if ( ! test -e $dest ) || ( test $i -nt $dest ) 
		then
			$exec_timeout $timeout $build_farbank $i $dest
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
