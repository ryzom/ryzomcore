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
# Ryzom patch shard ribs
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
	ribs_script "shard/ryzom_patch_shard.sh" "-Patch Shard"
	end_prepare
fi

if ribs_prepare $1
then
	end_prepare
fi

clr_brown "Updating $RYZOMDATA_PATH..."
cd $RYZOMDATA_PATH
hg pull
hg -v update
check_success

clr_brown "Updating $RYZOMSERVERDATA_PATH..."
cd $RYZOMSERVERDATA_PATH
hg pull
hg -v update
check_success

clr_green "Generating sheet_id.bin..."
LEVELDESIGN_PATH=$RYZOMSERVERDATA_PATH/common/data_leveldesign
FILESHEET=sheet_id.bin
make_sheet_id -o$LEVELDESIGN_PATH/leveldesign/Game_elem/$FILESHEET \
	$LEVELDESIGN_PATH/leveldesign/Game_elem \
	$LEVELDESIGN_PATH/leveldesign/game_element \
	$LEVELDESIGN_PATH/leveldesign/World \
	$LEVELDESIGN_PATH/leveldesign/ryzom \
	$RYZOMSERVERDATA_PATH/data/mirror_sheets
check_success

clr_brown "Cleaning Packed Sheets..."
rm $SHARD_PATH/data/leveldesign/*.packed_sheets
check_success

