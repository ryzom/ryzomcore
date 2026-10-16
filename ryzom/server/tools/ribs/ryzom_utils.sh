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
# Ryzom utils
#
function check_git_repository {
	if [[ ! -d $1 ]]
	then
		p_error "Repository $1 don't exists..."
		exit 1
	else
		cd $1
		git fetch
		BRANCH=$(git rev-parse --abbrev-ref HEAD)
		if [[ -z "$BRANCH" ]]
		then
			p_error "Repository $1 don't works..."
			exit 1
		else
			MODIFIED_FILES=$(git ls-files -m)
			DELETED_FILES=$(git ls-files -d)
			REMOTE=$(git rev-parse @{u})
			LOCAL=$(git rev-parse HEAD)
			ID=$(git rev-list HEAD --count)

			if [ "$LOCAL" == "$REMOTE" ]
			then
				if [[ "$2" == "-" ]]
				then
					echo "OK"
				else
					p_ok "\[$(p_value "$BRANCH")] $1 $(p_value "v$ID")"
				fi
			else
				if [[ "$2" == "-" ]]
				then
					echo "KO"
				else
					p_error "\[$BRANCH] $1 $(p_value "v$ID")"
					ask "Repo outdated: $(p_value "$(basename $1)")"
					select_list "No:Keep outdated" "+$2:Update repository"
				fi
			fi
			
			[ ! -z "$MODIFIED_FILES" ] && p_warning "Modified files:" && echo "$MODIFIED_FILES"
			[ ! -z "$DELETED_FILES" ] && p_warning "Deleted files:" && echo "$DELETED_FILES"
		fi
	fi
}


function update_git_repository {
	cd $1
	git pull
}

function get_client_version {
	echo $(strings $1 | grep "$DOMAIN v[2-9][0-9].[0-9][0-9].[1-9][0-9]* #[0-9a-f]*" | cut -d" " -f3 | cut -d"." -f3)
}
