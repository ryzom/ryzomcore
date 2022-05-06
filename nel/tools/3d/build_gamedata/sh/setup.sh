#!/bin/bash
rm log.log 2> /dev/null

# Setup the processes

# Get the process list
process_to_complete=`cat cfg/config.cfg | grep "process_to_complete" | sed -e 's/process_to_complete//' | sed -e 's/ //g' | sed -e 's/=//g' | sed -e 's/,/ /g'`

# Get the update directory
update_directory=`cat cfg/config.cfg | grep "update_directory" | sed -e 's/update_directory//' | sed -e 's/ //g' | sed -e 's/=//g' | sed -e 's/,/ /g'`

# Get the database directory
database_directory=`cat cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the database letter
database_letter=`cat cfg/site.cfg | grep "database_letter" | sed -e 's/database_letter//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the database server
database_server=`cat cfg/site.cfg | grep "database_server" | sed -e 's/database_server//g' | sed -e 's/ //g' | sed -e 's/=//g'`

`cat _idle.bat | sed -e "s&database_directory&$database_directory&g" | sed -e "s&database_letter&$database_letter&g" | sed -e "s&database_server&$database_server&g" > idle.bat`

# Log error
echo  > log.log
date >> log.log
date

# Create a bin dir
mkdir bin 2> /dev/null

# For each process
for i in $process_to_complete ; do
	# Open the directory
	cd processes/$i

	# Excecute the command
	./0_setup.bat

	# Get back
	cd ../..

	# Concat log.log files
	# cat processes/$i/log.log >> log.log

	# Idle
	./idle.bat
done

# Get the quality option to choose the goor properties.cfg file
quality_flag=`cat cfg/site.cfg | grep "build_quality" | grep "1"`

# Copy the good properties.cfg file
if ( test "$quality_flag" )
then
	# We are in BEST mode
	echo [Quality] BEST
	cp cfg/properties_final.cfg cfg/properties.cfg
else
	# We are not DRAFT mode
	echo [Quality] DRAFT
	cp cfg/properties_draft.cfg cfg/properties.cfg
fi
