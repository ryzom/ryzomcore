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
			if [[ "$NAME" == "ais_fyros" ]] || [[ "$NAME" == "ais_matis" ]] || [[ "$NAME" == "ais_tryker" ]] || [[ "$NAME" == "ais_roots" ]] || [[ "$NAME" == "ais_zorai" ]] || [[ "$NAME" == "ais_ark" ]]
			then
				touch "$SHARD_PATH/logs/ai_service_${NAME}.log"
				"$CWD/wait_and_notify.sh" "$SHARD_PATH/logs/ai_service_${NAME}.log" "[[ARK]] AIS UP" $NAME 0 &
				sleep 4
			elif [[ "$NAME" == "egs" ]]
			then
				touch "$SHARD_PATH/logs/entities_game_service.log"
				"$CWD/wait_and_notify.sh" "$SHARD_PATH/logs/entities_game_service.log" "onAiInstanceReady :  AI Instance 1 is up" $NAME 0 &
				sleep 2
			elif [[ "$NAME" == "ios" ]]
			then
				touch "$SHARD_PATH/logs/input_output_service.log"
				"$CWD/wait_and_notify.sh" "$SHARD_PATH/logs/input_output_service.log" "cbDynChatAddChan: add channel : FACTION_MARAUDER" $NAME 0 &
				sleep 2
			elif [[ "$NAME" == "gpms" ]]
			then
				touch "$SHARD_PATH/logs/gpm_service.log"
				"$CWD/wait_and_notify.sh" "$SHARD_PATH/logs/gpm_service.log" "cbCreateIndoorUnit : MSG: Creating indoor unit 256" $NAME 0 &
				sleep 2
			else
				declare -A ServiceLogs
				ServiceLogs[aes] = "admin_executor_service.log"
				ServiceLogs[bms_master] = "backup_service.log"
				ServiceLogs[fes] = "frontend_service.log"
				ServiceLogs[las] = "log_analyser_service.log"
				ServiceLogs[lgs] = "logger_service.log"
				ServiceLogs[mos] = "monitor_service.log"
				ServiceLogs[ms] = "mirror_service.log"
				ServiceLogs[ras] = "admin_service.log"
				ServiceLogs[rns] = "naming_service.log"
				ServiceLogs[rws] = "welcome_service.log"
				ServiceLogs[sbs] = "session_browser_server.log"
				ServiceLogs[su] = "shard_unifier_service.log"
				ServiceLogs[ts] = "tick_service.log"

				touch "$SHARD_PATH/logs/${ServiceLogs[$NAME]}"
				"$CWD/wait_and_notify.sh" "$SHARD_PATH/logs/${ServiceLogs[$NAME]}" "SERVICE: Service ready" $NAME 2 &
			fi
			
			export LC_ALL=C; unset LANGUAGE

			if [[ "$USE_GDB" == "1" ]]
			then
				if [[ "$NAME" == "egs" ]] || [[ "$NAME" == "ios" ]] || [[ "$NAME" == "ais_fyros" ]] || [[ "$NAME" == "ais_matis" ]] || [[ "$NAME" == "ais_tryker" ]] || [[ "$NAME" == "ais_roots" ]] || [[ "$NAME" == "ais_zorai" ]] || [[ "$NAME" == "ais_ark" ]] || [[ "$NAME" == "gpms" ]]
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

