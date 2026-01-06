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
# Copyright (C) 2025 Nuneo (ulukyn@gmail.com)
#
# This program is free software (GPLv3): read https://www.gnu.org/licenses/gpl-3.0.en.html for more details
#
# This Script start a python script in a loop with:
# - A secure check of 5s before 2 runs
# - A monitoring who restart the script if sources files changed (not otpimal but working)

SCRIPT_DIR=$(cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd)
CURRENT_PID=$$
echo "--- $CURRENT_PID ---"

cd $WORKING_DIR

echo "LOOP"
while true
do
	echo "Starting script : $*"
	echo "------------------"
	date
	echo "------------------"
	python3 $*
	#ps aux | grep -v grep | grep "python3 $*"
	#echo "Looking for Running script $*:"
	#ps aux | grep -v grep | grep "python3 $*"
	#for PID in $(ps aux | grep -v grep | grep "python3 $*" | tr -s " " | cut -d" " -f2)
	#do
		#echo "Found it! Killing $PID..."
		#kill -9 $PID
	#done

	#echo "Starting monitoring of python script..."
	#bash $SCRIPT_DIR/monitor_python.sh "python3 $*" $CURRENT_PID &

	#echo "Starting python3 $*..."
	#DATE_START=$(date +%s)

	#python3 $* 1>/dev/null

	#DATE_END=$(date +%s)
	#DATE_DIFF=$(expr $DATE_END - $DATE_START)
	#echo "End of script : $DATE_DIFF"
	#echo "Kill monitor python..."
	#ps aux | grep -v grep | grep "inotifywait" | grep "$SCRIPT_DIR"

	#for PID in $(ps aux | grep -v grep | grep "inotifywait " | grep "$SCRIPT_DIR" | tr -s " " | cut -d" " -f2)
	#do
			#echo "Killing $PID..."
			#kill -9 $PID
	#done

	#if (( $DATE_DIFF < 5 ))
	#then
		#echo "sleep $(expr 5 - $DATE_DIFF)"
		#sleep $(expr 5 - $DATE_DIFF)
	#fi
	echo "--------- END ---------"
	echo ""
	echo ""
done

