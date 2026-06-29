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
# Ryzom ribs utils
#

. $RIBS_PATH/colors.sh

function ribs_check {
	if [[ "$?" != "0" ]]
	then
		exit
	fi
}

function ribs_recipe {
	str=""
	for i in "$@"; do
		str="$str|$i"
	done
	echo ":>recipe:"${str:1}
}

function ribs_script {
	str=""
	for i in "$@"; do
		str="$str,$i"
	done
	echo -n ${str:1}
}

function ribs_prepare {
	if [[ "$1" == "Run" ]]
	then
		return 1
	else
		return 0
	fi
}

function ask {
	echo ":>ask:$@"
}

function enter_text {
	echo ":>input:$1|$2"
}

function select_list {
	echo ":>select:$1|$2|$3|$4|$5"
}

function p_title {
	echo ""
	echo "[cyan bold]⚜️⚜️⚜️  $1 ⚜️⚜️⚜️️[/]"
}

function p_action {
	echo "[orange1]$1[/orange1]"
}

function p_value {
	echo "[gold1]$1[/gold1]"
}


function p_badvalue {
	echo "[red]$1[/red]"
}

function p_info {
	echo "[bright_white]ℹ️  $1[/bright_white]"
}

function p_ok {
	echo "[green]✅  $1[/green]"

}

function p_error {
	echo "[red]📛  $1[/red]"
}

function p_warning {
	echo "[orange1]⚠️  $1[/orange1]"
}

function print_percent {
	PERCENT=$(python -c "print('{0:0=3d}'.format(round(100*$1/$2)))")
	echo "[$PERCENT%]"
}


########################################################################


declare -A OPTIONS
declare -A RECIPE_IDS

# if $1 not null, get all params from it
if [ ! -z "$1" ]
then
	for i in "$@"; do
		parameter=$(echo "$i" | cut -d"=" -f1)
		value=$(echo "$i" | cut -s -d"=" -f2-)
		if [ -z "$value" ]
		then
			parameter="$i"
			value="1"
		fi
		#echo "[$parameter]=$value"
		OPTIONS[$parameter]="$value"
	done
fi


function check_running_ribs {
	RIBSNAME=$(echo $1 | sed 's#/#__#g')
	RET=$(screen -list | grep \\\.RIBS_$RIBSNAME | wc -l)
	if [[ "$RET" == "1" ]]
	then
		clr_red "RIBS $1 ALLREADY RUNNING! COOKING ABORTED"
	fi
}

function enter_password {
	if [ "$RIBS_CONTEXT" == "WEB" ]
	then
		echo "(>password)$1"
	else
		read VALUE
		OPTIONS[$1]="$VALUE"
	fi
}

function script_name {
	echo $(basename $0 .sh)
}

function newline {
	echo "(>text) "
}


function clean {
	if [ "$RIBS_CONTEXT" == "WEB" ]
	then
		echo "(>clear) "
	fi
}

function refresh {
	if [ "$RIBS_CONTEXT" == "WEB" ]
	then
		echo "(>refresh) "
	fi
}

function reprepare {
	if [ "$RIBS_CONTEXT" == "WEB" ]
	then
		echo "(>reprepare) "
	fi
}


function login {
	if [ "$RIBS_CONTEXT" == "WEB" ]
	then
		echo "(>login)$1"
	fi
}

function display_user {
	user=$(clr_green "$RIBS_USER")
	groups=$(echo "$RIBS_GROUPS" | sed "s/:/ /g")
	clr_brown "=============================================================================================="
	clr_white "User:   " $user
	clr_white "Groups:" $(clr_green "$groups")
	clr_brown "=============================================================================================="
}

function logged {
	if [ "$RIBS_CONTEXT" == "WEB" ]
	then
		echo "(>logged)$1"
	fi
}


function ribs_flavour {
	str=""
	for i in "$@"; do
		str="$str|$i"
	done
	echo "(>flavour)"${str:1}
}

function get_recipe {
	if [ "$RIBS_CONTEXT" == "WEB" ]
	then
		export RECIPE_NAME=$1
		clr_brown "Getting recipe $(clr_cyan ${1:0:-3})"
	fi
	. $1
}





function ribs_recipe_icons {
	icon=$1
	name=$2
	shift 2
	str="<img title='$name' src='/static/images/$icon.png' />"
	for i in "$@"; do
		str="$str|$i"
	done
	echo "(>recipe)"${str}
}

function end_prepare {
	exit
}

function start_block {
	echo "(>start_block)"
}

function end_block {
	echo "(>end_block)"
}

function ok {
	clr_green "finished!"
}

function check_success {
	if [ $? -eq 0 ]
	then
		clr_brown "=== done ======================================================================================="
		if [ ! -z "$1" ]
		then
			p_info "End session $1..."
			schroot --end-session -c "$1"
		fi
	else
		clr_red "=== Command Failed! ============================================================================"
		if [ ! -z "$1" ]
		then
			p_info "End session $1..."
			schroot --end-session -c "$1"
		fi
		echo "(>fail)"
		exit
	fi
}



function checkPackage {
	cd "$SHARD_PATH/data"
	clr_green "Downloading $1..."
	wget -c --quiet "http://data.ryzom.com/shard_datas/$1.tgz" || exit
	clr_green "Checking package integrity..."
	# check accidental corruption
	if [ ! -f "$SHARD_PATH/data/SHA256" ]
	then
		cp "$RIBS_BASE_PATH/data/SHA256" "$SHARD_PATH/data/"
	fi
	sha256sum -c --ignore-missing SHA256 2>/dev/null
	if [ $? -eq 0 ]
	then
		clr_green "Unpacking $1..."
		tar xzf "$1.tgz"
		rm -f "$1.tgz"
	else
		clr_red "Corrupted!"
		exit
	fi
}

function escape {
	echo "$1" | sed -e 's/"/\\"/g'
}

function sanitize {
	echo -E "$1" | sed "s/'//g;s/\"//g;s/\\\//g"
}

function get_ini_var {
	sed -nr "/^\[$1\]/ { :l /^$2[ ]*=/ { s/[^=]*=[ ]*//; p; q;}; n; b l;}" $3
}




