#!/bin/bash
#############################################
#   __   __  _______  ______    _______  __   __  _______  _______ 
#  |  |_|  ||   _   ||    _ |  |       ||  | |  ||       ||       |
#  |       ||  |_|  ||   | ||  |    ___||  | |  ||    ___||____   |
#  |       ||       ||   |_||_ |   | __ |  |_|  ||   |___  ____|  |
#  |       ||       ||    __  ||   ||  ||       ||    ___|| ______|
#  | ||_|| ||   _   ||   |  | ||   |_| ||       ||   |___ | |_____ 
#  |_|   |_||__| |__||___|  |_||_______||_______||_______||_______|
# 
# M.A.R.G.U.E.Z (The Dyslexic Merguez)
# Copyright (C) 2025 Nuneo (nuno@troispetits.net)
#
# This program is free software (GPLv3): read https://www.gnu.org/licenses/gpl-3.0.en.html for more details
#
# This Script check if a new marguez script must be run and clean it after

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
SERVS=$SCRIPT_DIR/servs

cd ..

while true
do
	# Start App
	for serv in $SERVS/*.cmd
	do
		if [ $(basename $serv) == "*.cmd" ]
		then
			continue
		fi
		
		SID=$(basename -s .cmd $serv)
		rm $SERVS/$SID.cmd
		$HOME/.local/bin/textual-web -r "python3 shard.py" -e local | tee $SERVS/$SID.log &
		echo $! > $SERVS/$SID.pid
		PID=$(cat $SERVS/$SID.pid)
		echo "Started shard.py Webized App: $SID = $PID"
	done 
	sleep 1

	# Check end of App
	for serv in $SERVS/*.pid
	do
		if [ $(basename $serv) == '*.pid' ]
		then
			continue
		fi
		
		SID=$(basename -s .pid $serv)
		PID=$(cat $serv | head -n 1)
		COUNT=$(cat $serv | wc -l)
		echo "$PID" >> $serv
		if (( $COUNT >= 20 ))
		then
			STATUS=$(grep "'python3 shard.py'" $SERVS/$SID.log | wc -l)
			if [ $((STATUS%2)) -eq 0 ]
			then
				echo "App $SID timed out. Clean it."
				PID=$(cat $serv)
				kill $PID
				rm $SERVS/$SID.*
			fi
		fi
	done
done
