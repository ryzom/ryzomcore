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

CWD=$(dirname "$0")
LOGFILE=$1
STRING=$2
NAME=$3
SLEEP=$4

$CWD/notify.sh ServiceStarting $NAME
mkdir -p $SHARD_PATH/states/
sleep $SLEEP
grep -m 1 "$STRING" <( exec tail -f $LOGFILE )

echo "notifying $CWD/notify.sh ServiceStarted $NAME"
$CWD/notify.sh ServiceStarted $NAME


