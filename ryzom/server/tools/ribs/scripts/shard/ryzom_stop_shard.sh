#!/bin/bash
#############################################
# | ___ \|_   _|| ___ \/  ___|
# | |_/ /  | |  | |_/ /\ `--.
# |    /   | |  | ___ \ `--. \
# | |\ \ __| |__| |_/ //\__/ /
# \_| \_(_)___(_)____(_)____(_)
#
# Remote Interface to Bash Scripts
# - better with barcebue sauce -
# Copyright (C) 2019 Nuneo (nuno@troispetits.net)
#
# This program is free software (GPLv3): read https://www.gnu.org/licenses/gpl-3.0.en.html for more details
#
# Ryzom stop shard ribs
#

. $RIBS_PATH/utils.sh
. $RIBS_PATH/ryzom_prefs.sh
. $RIBS_PATH/shard_stop_prefs.sh

if ( [[ "$SHARD_TYPE" != "live" ]] && [[ "$RIBS_GROUPS" == *":leveldesigner:"* ]] ) || [[ "$RIBS_GROUPS" == *":dev:"* ]]
then

	if [[ -z "$*" ]]
	then
		ribs_script "shard/ryzom_stop_shard.sh" "Stop Shard"
		end_prepare
	fi

	if ribs_prepare $1
	then
		if [[ ! -z "$GET_CONNECTED_COMMAND" ]]
		then
			$GET_CONNECTED_COMMAND

			if [[ $? == 0 ]]
			then
				ask "Players connected. What to do?"
				if [[ -z "$BROADCAST_COMMAND" ]] || [[ "$SHARD_TYPE" != "live" ]]
				then
					select_list "+kickall:Kick them all" "forget:Abort stop process"
				else
					select_list "kickall:Kick them all" "+broadcast:Yes but broadcast before..." "forget:Abort stop process"
				fi
			else
				p_warning $output
			fi
		fi
		end_prepare
	fi

	if [ "${OPTIONS[forget]}" == "1" ]
	then
		exit
	fi

	p_action "Lock access..."
	php "$WEB_PATH/tools/manage_shard.php" lock
	echo "ds_dev" > /home/nevrax/www/login/server_open_status

	if [ "${OPTIONS[broadcast]}" == "1" ]
	then
		if [[ ! -z "$BROADCAST_COMMAND" ]]
		then
			echo "Broadcasting..."
			timer=$($BROADCAST_COMMAND | tail -n 1)
			echo "timer = $timer"
			for i in {1..10}
			do
				echo "Broadcast $i/10"
				sleep $timer
			done
		fi
	fi

	php "$WEB_PATH/tools/manage_shard.php" kick_them_all
	sleep 1

	p_action "Stopping Shard..."

	bash $SHARD_PATH/tools/notify.sh ShardStopped

	php "$WEB_PATH/tools/manage_shard.php" stopEgs
	sleep 1

	bash $SHARD_PATH/tools/shard.sh stop $SHARD_PATH
	sleep 2

	p_action "Cleaning schroots..."
	for schroot in $(mount | grep /run/schroot/mount/ | grep /home | cut -d/ -f 7)
	do
		echo $schroot;
		schroot --end-session -c $schroot 2> /dev/null
	done

	if [ "$BACKUP_LOGS" == "1" ]
	then
		DATE=$(date +%Y-%m-%d-%T)
		mkdir -p $SHARD_PATH/backups/logs/$DATE
		cp $SHARD_PATH/logs/* $SHARD_PATH/backups/logs/$DATE
	fi

	p_action "Removing logs..."
	rm -f $SHARD_PATH/logs/*
	rm -f $SHARD_PATH/logs/chat/chat???.log
	echo "" > $SHARD_PATH/logs/chat/chat.log

	echo "ds_closed" > /home/nevrax/www/login/server_open_status

	#send_chat SERVER_REBOOT "" "*Server*: Stopped!" ":inbox_tray:" "#FF5F5F"
fi
