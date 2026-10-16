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
# Ribs Ryzom patch live client recipe
#

. $RIBS_PATH/utils.sh
. $RIBS_PATH/ryzom_prefs.sh

if [[ "$RIBS_GROUPS" == *":dev:"* ]]
then
	if [[ "$SHARD_TYPE" == "staging" ]]
	then
		CHECK_PATCH=$(bash $RIBS_PATH/scripts/data/check_patch_client.sh)
		PATCH=$(bash $RIBS_PATH/scripts/data/patch_client.sh)
		SEND_TO_WEB=$(bash $RIBS_PATH/scripts/data/send_to_web.sh)
		ribs_check "$CHECK_PATCH"
		ribs_check "$PATCH"
		ribs_check "$SEND_TO_WEB"
		ribs_recipe "🔥" "Patching Live" "$CHECK_PATCH" "$PATCH" "$SEND_TO_WEB"
	else
		p_info "Only for Staging"
	fi
else
	echo "no soup for you!"
fi
