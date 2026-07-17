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
# This Script monitor changes in sources files and kill the python process (not optimal but working)

SCRIPT_DIR=$(cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd)

echo -e "\t\tMONITHON: Waiting for process $1..."
PID=""

while [[ -z $PID ]]
do
	ps_out=$(ps aux | grep -v grep | grep -v monitor_ | grep "$1" | head -n1 | tr '\n' '\n\t\t')
	echo -e "\t\tMONITHON: $ps_out"
	PID=$(ps aux | grep -v grep | grep -v monitor_ | grep "$1" | head -n1 | tr -s " " | cut -d" " -f2)
	#PID=$(ps aux | grep -v grep | grep "$1" | tr -s " " | cut -d" " -f2)
	sleep 0.5
done
echo -e "\t\tMONITHON: Process found: $PID!"
echo -e "\t\tMONITHON: inotifywait -q -e modify -r $SCRIPT_DIR $WORKING_DIR --> kill $PID"
inotifywait -q -e modify --include '.*\.py$' -r $SCRIPT_DIR .
echo -e "\t\tMONITHON: Killing $PID...."
kill $PID
