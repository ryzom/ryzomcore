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
# Copyright (C) 2019 Nuneo (ulukyn@gmail.com)
#
# This program is free software (GPLv3): read https://www.gnu.org/licenses/gpl-3.0.en.html for more details
#
# Ryzom manage shard recipe
#

. $RIBS_PATH/utils.sh
. $RIBS_PATH/ryzom_prefs.sh

if ( [[ "$SHARD_TYPE" != "live" ]] && [[ "$RIBS_GROUPS" == *":leveldesigner:"* ]] ) || [[ "$RIBS_GROUPS" == *":dev:"* ]]
then
	STOP=$(bash $RIBS_PATH/scripts/shard/ryzom_stop_shard.sh)
	START=$(bash $RIBS_PATH/scripts/shard/ryzom_start_shard.sh)
	ribs_check "$STOP"
	ribs_check "$START"

	if [[ "$SHARD_TYPE" == "live" ]]
	then
		OPEN=$(bash $RIBS_PATH/scripts/shard/ryzom_open_shard.sh)
		PATCH=$(bash $RIBS_PATH/scripts/shard/ryzom_patch_shard.sh)
		ribs_check "$OPEN"
		ribs_check "$PATCH"
		ribs_recipe "📟" "Manage Shard" "$STOP" "$PATCH" "$START" "$OPEN"
	else
		ribs_recipe "📟" "Manage Shard" "$STOP" "$START"
	fi
else
	echo "No soup for you"
fi
