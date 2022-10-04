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
# This script will notify external services like RocketChat or Web Apps
#

COMMAND=$1

CWD=$(dirname "$0")
. "$CWD/config.sh"

echo $COMMAND
if [[ "$COMMAND" == "ServiceStarted" ]]
then
	if [[ ! -z "$NOTIFY_URL_SERVICE_RESTARTED" ]]
	then
		echo "nofitiy $(hostname -s):$2 started"
		curl "$NOTIFY_URL_SERVICE_RESTARTED?command=started&shard=$(hostname -s)&apikey=$NOTIFY_URL_KEY&service=$2" & 2> /dev/null
	fi
elif [[ "$COMMAND" == "ServiceStopped" ]]
then
	if [[ ! -z "$NOTIFY_URL_SERVICE_RESTARTED" ]]
	then
		echo "nofitiy $(hostname -s):$2 stopped"
		curl "$NOTIFY_URL_SERVICE_RESTARTED?command=stopped&shard=$(hostname -s)&apikey=$NOTIFY_URL_KEY&service=$2" & 2> /dev/null
	fi
elif [[ "$COMMAND" == "ShardStopped" ]]
then
	if [[ ! -z "$NOTIFY_URL_SERVICE_RESTARTED" ]]
	then
		echo "nofitiy $(hostname -s):$2 shard stopped"
		curl "$NOTIFY_URL_SERVICE_RESTARTED?command=shard_stopped&shard=$(hostname -s)&apikey=$NOTIFY_URL_KEY" & 2> /dev/null
	fi
elif [[ "$COMMAND" == "ShardStarted" ]]
then
	if [[ ! -z "$NOTIFY_URL_SERVICE_RESTARTED" ]]
	then
		echo "nofitiy $(hostname -s):$2 shard started"
		curl "$NOTIFY_URL_SERVICE_RESTARTED?command=shard_started&shard=$(hostname -s)&apikey=$NOTIFY_URL_KEY" & 2> /dev/null
	fi
fi

