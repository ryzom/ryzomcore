#!/bin/bash
# ______                           _____ _                   _   _____           _
# | ___ \                         /  ___| |                 | | |_   _|         | |
# | |_/ /   _ _______  _ __ ___   \ `--.| |__   __ _ _ __ __| |   | | ___   ___ | |___
# |    / | | |_  / _ \| '_ ` _ \   `--. \ '_ \ / _` | '__/ _` |   | |/ _ \ / _ \| / __|
# | |\ \ |_| |/ / (_) | | | | | | /\__/ / | | | (_| | | | (_| |   | | (_) | (_) | \__ \
# \_| \_\__, /___\___/|_| |_| |_| \____/|_| |_|\__,_|_|  \__,_|   \_/\___/ \___/|_|___/
#        __/ |
#       |___/
#
# Ryzom - MMORPG Framework <https://ryzom.com/dev/>
# Copyright (C) 2019  Winch Gate Property Limited
# This program is free software: read https://ryzom.com/dev/copying.html for more details
#
# This script is a service launcher that works with a command file
# to determine when to launch the application that it is responsible for
#

CWD=$(dirname "$0")
. "$CWD/config.sh"

NAME="$1"
shift

EXECUTABLE=$1
shift

CTRL_CMDLINE=$*

mkdir -p $NAME

DOMAIN=shard
NAME_BASE="$NAME/$NAME"
CTRL_FILE=${NAME_BASE}_immediate.launch_ctrl
NEXT_CTRL_FILE=${NAME_BASE}_waiting.launch_ctrl
STATE_FILE=${NAME_BASE}.state
START_COUNTER_FILE=${NAME_BASE}.start_count

echo
echo ---------------------------------------------------------------------------------
echo Starting service launcher
echo ---------------------------------------------------------------------------------
printf "%-16s = " CMDLINE         ; echo $CTRL_CMDLINE
printf "%-16s = " CTRL_FILE       ; echo $CTRL_FILE
printf "%-16s = " NEXT_CTRL_FILE  ; echo $NEXT_CTRL_FILE
printf "%-16s = " STATE_FILE      ; echo $STATE_FILE
echo ---------------------------------------------------------------------------------
echo

# reinit the start counter
echo 0 > $START_COUNTER_FILE
START_COUNTER=0

while true
do
	# see if the conditions are right to launch the app
	if [ -e $CTRL_FILE ]
	then

		# a control file exists so read it's contents
		CTRL_COMMAND=_$(cat $CTRL_FILE)_

		# do we have a 'launch' command?
		if [ $CTRL_COMMAND = _LAUNCH_ ]
		then

			# update the start counter
			START_COUNTER=$(( $START_COUNTER + 1 ))
			echo $START_COUNTER > $START_COUNTER_FILE

			# we have a launch command so prepare, launch, wait for exit and do the housekeeping
			echo -----------------------------------------------------------------------
			echo Launching $(pwd) $EXECUTABLE $CTRL_CMDLINE...
			echo
			printf RUNNING > $STATE_FILE


			#notify start
			if [ "$NAME" = "egs" ] || [ "$NAME" = "ios" ] || [ "$NAME" = "gpms" ]
			then
				sleep 2
			elif [ "$NAME" = "ais_fyros" ] || [ "$NAME" = "ais_matis" ] || [ "$NAME" = "ais_tryker" ] || [ "$NAME" = "ais_roots" ] || [ "$NAME" = "ais_zorai" ] || [ "$NAME" = "ais_ark" ]
			then
				sleep 4
			fi

			echo "notifying $CWD/notify.sh ServiceStarted $NAME"
			"$CWD/notify.sh" ServiceStarted $NAME

			export LC_ALL=C; unset LANGUAGE

			if [[ "$USE_GDB" == "1" ]]
			then
				if [ "$NAME" = "egs" ] || [ "$NAME" = "ios" ] || [ "$NAME" = "ais_fyros" ] || [ "$NAME" = "ais_matis" ] || [ "$NAME" = "ais_tryker" ] || [ "$NAME" = "ais_roots" ] || [ "$NAME" = "ais_zorai" ] || [ "$NAME" = "ais_ark" ] || [ "$NAME" = "gpms" ]
				then
					echo "LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SHARD_PATH/lib gdb -batch -ex 'set logging file $NAME/gdb_dump.txt' -ex 'set logging on' -ex 'run $CTRL_CMDLINE' -ex 'bt' $EXECUTABLE" > /tmp/run_$NAME
				else
					echo "LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SHARD_PATH/lib $EXECUTABLE $CTRL_CMDLINE" > /tmp/run_$NAME
				fi

			else
				echo "LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SHARD_PATH/lib $EXECUTABLE $CTRL_CMDLINE" > /tmp/run_$NAME
			fi

			schroot -p -c atys -- sh /tmp/run_$NAME
			#notify stop
			"$CWD/notify.sh" ServiceStopped $NAME

			echo -----------------------------------------------------------------------
			printf STOPPED > $STATE_FILE

			# consume (remove) the control file to allow start once
			rm $CTRL_FILE

			if [[ "$AUTO_RESTART" == "0" ]]
			then
				echo "Press ENTER to relaunch"
				read
			fi
		fi
	fi

	# either we haven't launched the app yet or we have launched and it has exitted
	if [ -e $NEXT_CTRL_FILE ]
	then
		# we have some kind of relaunch directive lined up so deal with it
		mv $NEXT_CTRL_FILE $CTRL_FILE
	else
		# automatic launch
		printf LAUNCH > $CTRL_FILE
	fi
done

