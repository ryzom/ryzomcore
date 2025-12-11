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
# Ryzom start shard ribs
#

. $RIBS_PATH/utils.sh
. $RIBS_PATH/ryzom_utils.sh
. $RIBS_PATH/ryzom_prefs.sh

echo "-- $WEB_PATH --"
if [[ ! "$RIBS_GROUPS" == *":dev:"* ]]
then
	echo "No soup for you"
	exit
fi


if [[ -z "$*" ]]
then
	ribs_script "shard/ryzom_start_shard.sh" "Start Shard" "Silent" "CleanSheets"
	end_prepare
fi

if ribs_prepare $1
then
	check_git_repository $RYZOMSERVERDATA_PATH UpdateServerRepo

	if [ ! -f $SHARD_PATH/data/leveldesign/egs_items.packed_sheets ] || [ "${OPTIONS[CleanSheets]}" == "1" ]
	then
		p_warning "Packed Sheets are missing. Shard need 7min to regenerate them"
	fi

	if [ $SHARD_TYPE == "live" ]
	then
		bad_files=""
		for file in $(find /home/nevrax/shard/save -size 0)
		do
			if [ "$file" != "/home/nevrax/shard/save/new_save.txt" ] && [ "$file" != "/home/nevrax/shard/save/rrd_graphs/.xml" ] && [ "$file" != "/home/nevrax/shard/save/characters/account___pdr.bin" ]
			then
				bad_files="$bad_files$file<br />"
			fi
		done

		if [ ! -z "$bad_files" ]
		then
			p_error "!!!!!!!!!!!! WARNING !!!!!!!!!!!!"
			p_error "Bad files with size to 0:"
			echo " "
			echo "$bad_files"
			p_error "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
		fi
	fi

	if [ $SHARD_TYPE == "staging" ]
	then
		cd $RYZOMSERVERDATA_PATH/scripts/outpost/rotation/
		need_op_mats_rotation=$(python scripts/check_rotation.py)
		echo -n "Outpost mats rotation: "
		p_ok $need_op_mats_rotation

		if [ "$need_op_mats_rotation" == "Need" ]
		then
			# Get the csv used in Live
			scp app@app.ryzom.com:/home/app/www/app/data/app/new_outposts_atys.csv data/outposts.csv

			# Move current mats to old
			python scripts/switch_mats_in_csv.py

			# Made the rotation and update the csv
			python scripts/generate_mps.py

			# Update the buildings
			python scripts/update_drillers.py

			# Send the csv for the new rotation
			rotation=$(python -c 'import math, datetime; now = datetime.datetime.now(); print(1+int(math.floor(int(now.strftime("%m"))-1)/3))')
			scp data/outposts_$(date +%Y)_$rotation.csv app@app.ryzom.com:/home/app/www/app/data/app/outposts_$(date +%Y)_$rotation.csv

			# Next step are ingame using app_spawn_list
			# The app will display a button to finish the rotation
		fi
	fi

	end_prepare
fi

echo "ds_start" > /home/nevrax/www/login/server_open_status

if [ "${OPTIONS[UpdateServerRepo]}" == "1" ]
then
	update_git_repository $RYZOMSERVERDATA_PATH
fi

if  [ "${OPTIONS[CleanSheets]}" == "1" ]
then
	rm $SHARD_PATH/data/leveldesign/*.packed_sheets
fi

rm -f $SHARD_PATH/data/leveldesign/primitive_cache/*

D=$SHARD_PATH/language/translated/

p_action "Downloading translation files..."
for lang in en fr de es ru
do
	p_warningn "  Looking for $lang files:"
	if [ "$SHARD_DOMAIN" == "ryzom_dev" ]
	then
		wget -q https://me.ryzom.com/translations/item_words_${lang}_wk.txt -O $D/item_words_${lang}.txt
		p_ok "    Item Words: OK"
		wget -q https://me.ryzom.com/translations/creature_words_${lang}_wk.txt -O $D/creature_words_${lang}.txt
		p_ok "    Creature Words: OK"
		wget -q https://me.ryzom.com/translations/title_words_${lang}_wk.txt -O $D/title_words_${lang}.txt
		p_ok "    Title Words: OK"
		wget -q https://me.ryzom.com/translations/place_words_${lang}_wk.txt -O $D/place_words_${lang}.txt
		p_ok "    Place Words: OK"

		wget -q https://me.ryzom.com/translations/phrases_${lang}_wk.uxt -O $D/phrases_${lang}.uxt
		p_ok "    Phrases UXT: OK"

	else
		wget -q https://me.ryzom.com/translations/item_words_${lang}.txt -O $D/item_words_${lang}.txt
		p_ok "    Item Words: OK"
		wget -q https://me.ryzom.com/translations/creature_words_${lang}.txt -O $D/creature_words_${lang}.txt
		p_ok "    Creature Words: OK"
		wget -q https://me.ryzom.com/translations/title_words_${lang}.txt -O $D/title_words_${lang}.txt
		p_ok "    Title Words: OK"
		wget -q https://me.ryzom.com/translations/place_words_${lang}.txt -O $D/place_words_${lang}.txt
		p_ok "    Place Words: OK"

		wget -q https://me.ryzom.com/translations/phrases_${lang}.uxt -O $D/phrases_${lang}.uxt
		p_ok "    Phrases UXT: OK"
	fi

	wget -q https://app.ryzom.com/app_arcc/get_titles.php?lang=${lang} -O $D/title_ark_${lang}.txt
	cat $D/title_ark_${lang}.txt >> $D/title_words_${lang}.txt
	rm $D/title_ark_${lang}.txt
	p_ok "    Title ARK: OK"
done

wget -q https://me.ryzom.com/translations/item_words_wk.txt -O $D/item_words_wk.txt
p_ok "Item Words WK: OK"
wget -q https://me.ryzom.com/translations/creature_words_wk.txt -O $D/creature_words_wk.txt
p_ok "Creature Words WK: OK"
wget -q https://me.ryzom.com/translations/title_words_wk.txt -O $D/title_words_wk.txt
p_ok "Title Words WK: OK"
wget -q https://me.ryzom.com/translations/place_words_wk.txt -O $D/place_words_wk.txt
p_ok "Place Words WK: OK"
wget -q https://app.ryzom.com/app_arcc/get_titles.php?lang=en -O $D/title_ark_wk.txt
cat $D/title_ark_wk.txt >> $D/title_words_wk.txt
p_ok "Title ARK WK: OK"
wget -q https://me.ryzom.com/translations/bot_names.txt -O $D/bot_names.txt
p_ok "Bot Names: OK"

wget -q https://me.ryzom.com/translations/phrases_wk.uxt -O $D/phrases_wk.uxt
$D/update_phrases.py $D
p_ok "Merged Phrases: OK"

# This file is only required when updating primitives from cloud
rm -f /tmp/updating_primitives.lock

echo ""

bash $SHARD_PATH/tools/shard.sh start $SHARD_PATH

#bash $SHARD_PATH/tools/notify.sh ShardStarting

if [ ! -f $SHARD_PATH/data/leveldesign/egs_items.packed_sheets ]
then
	p_error "Missing packed_sheets. Will be regenerated..."
	p_warningn "Shard Started... wait 7 minutes"
else
	p_warningn "Shard Started... wait 2 minutes"
fi

p_action "Waiting for ready state..."
p_action "Start tailing and greping..."
touch $SHARD_PATH/logs/entities_game_service.log
grep -m 1 "onAiInstanceReady :  AI Instance 1 is up" <( exec tail -f $SHARD_PATH/logs/entities_game_service.log ); kill $!

p_ok "Shard is up!"

if [[ "$SHARD_TYPE" == "live" ]]
then
	if [ "${OPTIONS[Silent]}" == "1" ]
	then
		php "$WEB_PATH/tools/manage_shard.php" open silent
	else
		php "$WEB_PATH/tools/manage_shard.php" open
	fi
else
	php "$WEB_PATH/tools/manage_shard.php" open players
fi


#send_chat SERVER_REBOOT "" "*Server*: Started!" ":inbox_tray:" "#00FF00"
