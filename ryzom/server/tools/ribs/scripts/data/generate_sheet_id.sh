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
# Ryzom generate sheet_id.bin
#


. $RIBS_PATH/utils.sh
. $RIBS_PATH/ryzom_utils.sh
. $RIBS_PATH/ryzom_prefs.sh

if [[ ! "$RIBS_GROUPS" == *":dev:"* ]]
then
	exit
fi

if [[ -z "$*" ]]
then
	ribs_script "data/generate_sheet_id.sh" "Generate sheet_id"
	end_prepare
fi

if ribs_prepare $1
then
	#cd $RYZOMSERVERDATA_PATH
	#p_warning "Changes from Atys (master branch):"
	#git diff master yubo --stat common/data_leveldesign/leveldesign
	end_prepare
fi

FILESHEET=sheet_id.bin

if [ -e $RYZOMSERVERDATA_PATH/game_element/$FILESHEET ]
then
	cp $RYZOMSERVERDATA_PATH/game_element/$FILESHEET ~/tmp/$FILESHEET.old.`date +%s`
fi

if [[ "$SHARD_TYPE" == "test" ]]
then
	send_chat DATA_BUILD "" "*sheet_id.bin*: Generating..." ":book:" "#FFC200"
fi

p_ok "Generating sheet_id.bin..."
make_sheet_id -o$RYZOMSERVERDATA_PATH/game_element/$FILESHEET \
	$RYZOMSERVERDATA_PATH/game_element \
	$RYZOMDATA_PATH/leveldesign/world \
	$RYZOMSERVERDATA_PATH/data/mirror_sheets

p_ok "Remove all packed sheets from server"
rm -f $SHARD_PATH/data/leveldesign/*.packed_sheets

if [[ "$SHARD_TYPE" == "test" ]]
then
	send_chat DATA_BUILD "" "*sheet_id.bin*: Done" ":book:" "#00FF00"
fi
