#!/bin/bash
rm log.log 2> /dev/null

# Clean the processes

# Get the process list
process_to_complete=`cat cfg/config.cfg | grep "process_to_complete" | sed -e 's/process_to_complete//' | sed -e 's/ //g' | sed -e 's/=//g' | sed -e 's/,/ /g'`

# Log error
echo  > log.log

# For each process
for i in $process_to_complete ; do
	# Open the directory
	cd processes/$i

	# Excecute the command
	./1_clean.bat

	# Get back
	cd ../..

	# Concat log.log files
	# cat processes/$i/log.log >> log.log

	# Idle
	./idle.bat
done

