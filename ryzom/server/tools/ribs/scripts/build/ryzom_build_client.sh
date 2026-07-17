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
# Ryzom build client ribs
#

. $RIBS_PATH/utils.sh
. $RIBS_PATH/ryzom_utils.sh
. $RIBS_PATH/ryzom_prefs.sh
. $RIBS_PATH/ryzom_compilation_prefs.sh

if [[ ! "$RIBS_GROUPS" == *":dev:"* ]]
then
	echo "no soup for you!"
	exit
fi

if [[ -z "$*" ]]
then
	steam=""
	windows=""
	i386=""
	i64=""
	if [ "$USE_STEAM" == "1" ]
	then
		steam="Steam"
	fi
	if [ "$USE_WINE" == "1" ]
	then
		windows="+Windows"
	fi
	if [ "$USE_I386" == "1" ]
	then
		i386="32"
		i64="+64"
	fi

	ribs_script "build/ryzom_build_client.sh" "Build Client" "+Linux" "Macos" $windows $steam $i386 $i64 "Dev"
	exit
fi

if ribs_prepare $1
then
	p_title "Cheking repositories"
	check_git_repository "$RYZOMCORE_PATH" UpdateClientRepo
	
	if [ "$USE_CHROOT" == "1" ]
	then
		p_info "Compilation will use a $(p_value Chrooted) environement"
	fi

	LABEL=""

	if [ "${OPTIONS[Linux]}" == "1" ]
	then
		LABEL="$LABEL Linux"
	fi

	if [ "${OPTIONS[Windows]}" == "1" ]
	then
		LABEL="$LABEL Windows"
	fi
	
		if [ "${OPTIONS[Macos]}" == "1" ]
	then
		LABEL="$LABEL Macos"
	fi

	if [ "${OPTIONS[Dev]}" == "1" ]
	then
		LABEL="$LABEL (dev)"
	elif [ "${OPTIONS[Steam]}" == "1" ]
	then
		LABEL="$LABEL (steam)"
	else
		LABEL="$LABEL (fv)"
	fi

	if [ "${OPTIONS[32]}" == "1" ]
	then
		LABEL="$LABEL x32"
	fi

	if [ "${OPTIONS[64]}" == "1" ]
	then
		LABEL="$LABEL x64"
	fi

	p_info "System will compile for$(p_value "$LABEL")"

	ask What your name?
	enter_text Test "TESTING"

	exit
fi

#Update repositories
if [ "${OPTIONS[UpdateClientRepo]}" == "1" ]
then
	update_git_repository $RYZOMCORE_PATH
fi

function compile_clients()
{
	MODE=$1
	if [ "${OPTIONS[32]}" == "1" ]
	then
		if [ "${OPTIONS[Windows]}" == "1" ]
		then
			bash $RIBS_PATH/scripts/build/build_client.sh x86 Windows win $MODE client
		fi
	fi

	if [ "${OPTIONS[64]}" == "1" ] || [ "$USE_I386" == "0" ]
	then
		if [ "${OPTIONS[Linux]}" == "1" ]
		then
			bash $RIBS_PATH/scripts/build/build_client.sh amd64 GNU/Linux linux $MODE client
		fi


		if [ "${OPTIONS[Macos]}" == "1" ]
		then
			bash $RIBS_PATH/scripts/build/build_client.sh x64 Macos darwin $MODE client
		fi
		
		
		if [ "${OPTIONS[Windows]}" == "1" ]
		then
			bash $RIBS_PATH/scripts/build/build_client.sh x64 Windows win $MODE client
		fi
	fi
}

if [ "${OPTIONS[Steam]}" == "1" ]
then
	compile_clients "steam"
fi

if [ "${OPTIONS[Dev]}" == "1" ]
then
	compile_clients "dev"
else
	compile_clients "fv"
fi


