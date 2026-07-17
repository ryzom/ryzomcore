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
# This script will notify external services like Zulip or Web Apps
#

function get_ini_var {
	sed -nr "/^\[$1\]/ { :l /^$2[ ]*=/ { s/[^=]*=[ ]*//; p; q;}; n; b l;}" $3
}

COMMAND=$1

CWD=$(dirname "$0")
SHARD_PATH=$(get_ini_var shard path /etc/ryzom/shard.ini)
SHARD_WEB=$(get_ini_var notify token /etc/ryzom/shard.ini)
NOTIFY_URL_SERVICE_RESTARTED=$(get_ini_var notify url_services /etc/ryzom/shard.ini)
NOTIFY_URL_KEY=$(get_ini_var notify token /etc/ryzom/shard.ini)
STATUS=$(cat $SHARD_WEB/login/server_open_status)

if [[ "$COMMAND" == "ServiceStarted" ]]
then
	if [[ ! -z "$NOTIFY_URL_SERVICE_RESTARTED" ]]
	then
		curl --silent "$NOTIFY_URL_SERVICE_RESTARTED?command=started&shard=$(hostname -s)&apikey=$NOTIFY_URL_KEY&status=$STATUS&service=$2"
		echo $COMMAND > $SHARD_PATH/states/$2.txt
	fi
elif [[ "$COMMAND" == "ServiceStarting" ]]
then
	if [[ ! -z "$NOTIFY_URL_SERVICE_RESTARTED" ]]
	then
		curl --silent "$NOTIFY_URL_SERVICE_RESTARTED?command=starting&shard=$(hostname -s)&apikey=$NOTIFY_URL_KEY&status=$STATUS&service=$2"
		echo $COMMAND > $SHARD_PATH/states/$2.txt
	fi
elif [[ "$COMMAND" == "ServiceStopped" ]]
then
	if [[ ! -z "$NOTIFY_URL_SERVICE_RESTARTED" ]]
	then
		curl --silent "$NOTIFY_URL_SERVICE_RESTARTED?command=stopped&shard=$(hostname -s)&apikey=$NOTIFY_URL_KEY&status=$STATUS&service=$2"
		echo $COMMAND > $SHARD_PATH/states/$2.txt
	fi
elif [[ "$COMMAND" == "ShardStopped" ]]
then
	if [[ ! -z "$NOTIFY_URL_SERVICE_RESTARTED" ]]
	then
		curl --silent "$NOTIFY_URL_SERVICE_RESTARTED?command=shard_stopped&shard=$(hostname -s)&apikey=$NOTIFY_URL_KEY"
		echo $COMMAND > $SHARD_PATH/states/shard.txt
	fi
elif [[ "$COMMAND" == "ShardStarted" ]]
then
	if [[ ! -z "$NOTIFY_URL_SERVICE_RESTARTED" ]]
	then
		curl --silent "$NOTIFY_URL_SERVICE_RESTARTED?command=shard_started&shard=$(hostname -s)&apikey=$NOTIFY_URL_KEY"
		echo $COMMAND > $SHARD_PATH/states/shard.txt
	fi
elif [[ "$COMMAND" == "ShardStarting" ]]
then
	if [[ ! -z "$NOTIFY_URL_SERVICE_RESTARTED" ]]
	then
		curl --silent "$NOTIFY_URL_SERVICE_RESTARTED?command=shard_starting&shard=$(hostname -s)&apikey=$NOTIFY_URL_KEY"
		echo $COMMAND > $SHARD_PATH/states/shard.txt
	fi
fi

sleep 1
