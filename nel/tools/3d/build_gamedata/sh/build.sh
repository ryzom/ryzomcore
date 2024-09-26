#!/bin/bash
rm log.log 2> /dev/null

# Build the processes

# Get the process list
process_to_complete=`cat cfg/config.cfg | grep "process_to_complete" | sed -e 's/process_to_complete//' | sed -e 's/ //g' | sed -e 's/=//g' | sed -e 's/,/ /g'`

# Log error
echo  > log.log
date >> log.log
date

# For each process
for i in $process_to_complete ; do
	# Open the directory
	cd processes/$i

	# Excecute the command
	./3_build.bat

	# Get back
	cd ../..

	# Concat log.log files
	cat processes/$i/log.log >> log.log

	# Idle
	./idle.bat
done

# Copy the log file
cp log.log build.log
