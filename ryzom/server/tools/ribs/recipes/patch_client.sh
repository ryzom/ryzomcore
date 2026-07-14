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
# Ribs Ryzom patch client recipe
#

. $RIBS_PATH/utils.sh
. $RIBS_PATH/ryzom_prefs.sh

if ( [[ "$SHARD_TYPE" != "live" ]] && [[ "$RIBS_GROUPS" == *":leveldesigner:"* ]] ) || [[ "$RIBS_GROUPS" == *":dev:"* ]]
then
	SHEETID=$(bash $RIBS_PATH/scripts/data/generate_sheet_id.sh)
	CLIENTDATA=$(bash $RIBS_PATH/scripts/data/generate_client_data.sh)
	PATCH=$(bash $RIBS_PATH/scripts/data/patch_client.sh)
	ribs_check "$SHEETID"
	ribs_check "$CLIENTDATA"
	ribs_check "$PATCH"
	ribs_recipe "⚗️ " "Patch Client" "$SHEETID" "$CLIENTDATA" "$PATCH"
else
	echo "No soup for you!"
fi
