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
# Ryzom open shard ribs
#

. $RIBS_PATH/utils.sh
. $RIBS_PATH/ryzom_prefs.sh

if [[ ! "$RIBS_GROUPS" == *":dev:"* ]]
then
	echo "No soup for you"
	exit
fi

if [[ -z "$*" ]]
then
	ribs_script "shard/ryzom_open_shard.sh" "-Open Shard"
	end_prepare
fi

if ribs_prepare $1
then
	end_prepare
fi

php "$WEB_PATH/tools/manage_shard.php" open players
