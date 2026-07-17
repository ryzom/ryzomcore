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
# Ryzom build Server ribs
#

. $RIBS_PATH/utils.sh
. $RIBS_PATH/ryzom_utils.sh
. $RIBS_PATH/ryzom_prefs.sh
. $RIBS_PATH/ryzom_compilation_prefs.sh


if [[ "$RIBS_GROUPS" == *":dev:"* ]]
then

	SESSION_FILE="/tmp/build_server_chroot.session"
	
	if [[ -z "$*" ]]
	then
		ribs_script "build/ryzom_build_server.sh" "Build Server"
		end_prepare
	fi

	if [ "$1" == "stop" ]
	then
		if [ ! -z "$SERVER_CHROOT" ]
		then
			CHROOT_SESSION=$(cat "$SESSION_FILE")
			if [[ ! -z "$CHROOT_SESSION" ]]
			then
				schroot --end-session -c "$CHROOT_SESSION"
			fi
		fi
		exit
	fi


	if ribs_prepare $1
	then
		check_git_repository $RYZOMSERVER_PATH UpdateServerRepo
		exit
	fi

	CC="$SERVER_CC"
	CXX="$SERVER_CXX"
		
	#Update repositories
	if [ "${OPTIONS[UpdateServerRepo]}" == "1" ]
	then
		update_git_repository $RYZOMSERVER_PATH
	fi

	SBIN_PATH=$SHARD_PATH/sbin/

	xsltproc  --stringparam filename database_mapping --stringparam output header  --output $RYZOMSERVER_PATH/src/shard_unifier_service/database_mapping.h.tmp $RYZOMCORE_PATH/ryzom/common/src/game_share/generate_module_interface.xslt $RYZOMSERVER_PATH/src/shard_unifier_service/nel_database_mapping.xml
	xsltproc  --stringparam filename database_mapping --stringparam output cpp     --output $RYZOMSERVER_PATH/src/shard_unifier_service/database_mapping.cpp.tmp $RYZOMCORE_PATH/ryzom/common/src/game_share/generate_module_interface.xslt $RYZOMSERVER_PATH/src/shard_unifier_service/nel_database_mapping.xml

	xsltproc --stringparam filename database --stringparam bank PLR --stringparam output header --stringparam side server --output $RYZOMSERVER_PATH/src/entities_game_service/database_plr.h.tmp $RYZOMCORE_PATH/ryzom/common/src/game_share/generate_client_db.xslt $RYZOMCORE_PATH/ryzom/common/data_common/database.xml
	xsltproc --stringparam filename database --stringparam bank PLR --stringparam output cpp --stringparam side server --output $RYZOMSERVER_PATH/src/entities_game_service/database_plr.cpp.tmp $RYZOMCORE_PATH/ryzom/common/src/game_share/generate_client_db.xslt $RYZOMCORE_PATH/ryzom/common/data_common/database.xml

	xsltproc --stringparam filename database --stringparam bank GUILD --stringparam output header --stringparam side server --output $RYZOMSERVER_PATH/src/entities_game_service/database_guild.h.tmp $RYZOMCORE_PATH/ryzom/common/src/game_share/generate_client_db.xslt $RYZOMCORE_PATH/ryzom/common/data_common/database.xml
	xsltproc --stringparam filename database --stringparam bank GUILD --stringparam output cpp --stringparam side server --output $RYZOMSERVER_PATH/src/entities_game_service/database_guild.cpp.tmp $RYZOMCORE_PATH/ryzom/common/src/game_share/generate_client_db.xslt $RYZOMCORE_PATH/ryzom/common/data_common/database.xml

	xsltproc --stringparam filename guild_unifier_itf --stringparam bank GUILD --stringparam output cpp --stringparam side server --output  $RYZOMSERVER_PATH/src/entities_game_service/guild_manager/guild_unifier_itf.cpp.tmp $RYZOMCORE_PATH/ryzom/common/src/game_share/generate_module_interface.xslt $RYZOMSERVER_PATH/src/entities_game_service/guild_manager/guild_unifier_itf.xml
	xsltproc --stringparam filename guild_unifier_itf --stringparam bank GUILD --stringparam output header --stringparam side server --output  r$RYZOMSERVER_PATH/src/entities_game_service/guild_manager/guild_unifier_itf.h.tmp $RYZOMCORE_PATH/ryzom/common/src/game_share/generate_module_interface.xslt $RYZOMSERVER_PATH/src/entities_game_service/guild_manager/guild_unifier_itf.xml


	DIFF_DB_MAPPING_H=$(diff -w $RYZOMSERVER_PATH/src/shard_unifier_service/database_mapping.h.tmp $RYZOMSERVER_PATH/src/shard_unifier_service/database_mapping.h)
	DIFF_DB_MAPPING_CPP=$(diff -w $RYZOMSERVER_PATH/src/shard_unifier_service/database_mapping.cpp.tmp $RYZOMSERVER_PATH/src/shard_unifier_service/database_mapping.cpp)

	DIFF_DB_PLR_H=$(diff -w $RYZOMSERVER_PATH/src/entities_game_service/database_plr.h.tmp $RYZOMSERVER_PATH/src/entities_game_service/database_plr.h)
	DIFF_DB_PLR_CPP=$(diff -w $RYZOMSERVER_PATH/src/entities_game_service/database_plr.cpp.tmp $RYZOMSERVER_PATH/src/entities_game_service/database_plr.cpp)

	DIFF_DB_GUILD_H=$(diff -w $RYZOMSERVER_PATH/src/entities_game_service/database_guild.h.tmp $RYZOMSERVER_PATH/src/entities_game_service/database_guild.h)
	DIFF_DB_GUILD_CPP=$(diff -w $RYZOMSERVER_PATH/src/entities_game_service/database_guild.cpp.tmp $RYZOMSERVER_PATH/src/entities_game_service/database_guild.cpp)

	DIFF_GUILD_UNIFIER_H=$(diff -w $RYZOMSERVER_PATH/src/entities_game_service/guild_manager/guild_unifier_itf.h.tmp $RYZOMSERVER_PATH/src/entities_game_service/guild_manager/guild_unifier_itf.h)
	DIFF_GUILD_UNIFIER_CPP=$(diff -w $RYZOMSERVER_PATH/src/entities_game_service/guild_manager/guild_unifier_itf.cpp.tmp $RYZOMSERVER_PATH/src/entities_game_service/guild_manager/guild_unifier_itf.cpp)


	if [ ! -z "$DIFF_DB_MAPPING_H" ] || [ ! -z "$DIFF_DB_MAPPING_CPP" ]
	then
		p_ok "Generating database_guild..."
		cp $RYZOMSERVER_PATH/src/shard_unifier_service/database_mapping.h.tmp $RYZOMSERVER_PATH/src/shard_unifier_service/database_mapping.h
		cp $RYZOMSERVER_PATH/src/shard_unifier_service/database_mapping.h.tmp $RYZOMSERVER_PATH/src/shard_unifier_service/database_mapping.h
	fi

	if [ ! -z "$DIFF_DB_PLR_H" ] || [ ! -z "$DIFF_DB_PLR_CPP" ]
	then
		p_ok "Generating database_plr..."
		cp $RYZOMSERVER_PATH/src/entities_game_service/database_plr.h.tmp $RYZOMSERVER_PATH/src/entities_game_service/database_plr.h
		cp $RYZOMSERVER_PATH/src/entities_game_service/database_plr.cpp.tmp $RYZOMSERVER_PATH/src/entities_game_service/database_plr.cpp
	fi

	if [ ! -z "$DIFF_DB_GUILD_H" ] || [ ! -z "$DIFF_DB_GUILD_CPP" ]
	then
		p_ok "Generating database_guild..."
		cp $RYZOMSERVER_PATH/src/entities_game_service/database_guild.h.tmp $RYZOMSERVER_PATH/src/entities_game_service/database_guild.h
		cp $RYZOMSERVER_PATH/src/entities_game_service/database_guild.cpp.tmp $RYZOMSERVER_PATH/src/entities_game_service/database_guild.cpp
	fi

	if [ ! -z "$DIFF_GUILD_UNIFIER_H" ] || [ ! -z "$DIFF_GUILD_UNIFIER_CPP" ]
	then
		p_ok "Generating guild_unifier_itf..."
		cp $RYZOMSERVER_PATH/src/entities_game_service/guild_manager/guild_unifier_itf.h.tmp $RYZOMSERVER_PATH/src/entities_game_service/guild_manager/guild_unifier_itf.h
		cp $RYZOMSERVER_PATH/src/entities_game_service/guild_manager/guild_unifier_itf.cpp.tmp $RYZOMSERVER_PATH/src/entities_game_service/guild_manager/guild_unifier_itf.cpp
	fi

	# xsltproc  --output msg_ais_egs_gen.h generate_module_interface.xslt msg_ais_egs_gen.xml

	TYPE=$SERVER_TYPE
	PROC=$(nproc)

	p_info "Compilation params : (-j$PROC) $SHARD_NAME $TYPE"

	NEL_PATH=$SERVER_NEL_PATH/${SHARD_NAME}_${TYPE}

	DIR=$BUILD_PATH/server/${SHARD_NAME}_${TYPE}
	mkdir -p $DIR
	cd $DIR


	CMAKEFLAGS="-DCMAKE_C_COMPILER=$CC \
		-DCMAKE_CXX_COMPILER=$CXX \
		-DCMAKE_BUILD_TYPE=Release \
		-DWITH_NEL=ON \
		-DWITH_RYZOM_SERVER=ON \
		-DWITH_RYZOM_GAMESHARE=ON \
		-DWITH_SYMBOLS=ON \
		-DWITH_RYZOM_TOOLS=OFF \
		-DWITH_RYZOM_CLIENT=OFF \
		-DWITH_NEL_TESTS=OFF \
		-DWITH_NEL_TOOLS=OFF \
		-DWITH_NEL_SAMPLES=OFF \
		-DWITH_UNIX_STRUCTURE=OFF \
		-DWITH_INSTALL_LIBRARIES=OFF \
		-DWITH_DRIVER_OPENGL=OFF \
		-DWITH_DRIVER_OPENAL=OFF \
		-DWITH_PCH=OFF \
		-DWITH_STATIC=ON \
		-DWITH_GUI=OFF \
		-DWITH_3D=OFF \
		-DWITH_SOUND=OFF \
	"

	p_title "Runing CMake for Server from $RYZOMCORE_PATH..."
	p_info "Chroot: $SERVER_CHROOT";
	if [ ! -z "$SERVER_CHROOT" ]
	then
		CHROOT_SESSION=$(schroot --begin-session -c $SERVER_CHROOT)
		echo "$CHROOT_SESSION" > "$SESSION_FILE"
		p_action "Starting chroot session $CHROOT_SESSION"
		schroot --run-session -c "$CHROOT_SESSION" -- cmake $RYZOMCORE_PATH -DPLATFORM_CXXFLAGS="--param ggc-min-expand=1 --param ggc-min-heapsize=512000" $CMAKEFLAGS
		check_success
		p_action "Compiling..."
		schroot --run-session -c "$CHROOT_SESSION" -- make -j$PROC
		check_success "$CHROOT_SESSION"
		echo "" > "$SESSION_FILE"
	else
		echo "NO CHROOT"
		cmake $RYZOMSERVER_PATH $CMAKEFLAGS
		check_success
		p_action "Compiling..."
		make -j$PROC
		check_success
	fi

	p_action "Instaling from $(pwd)/bin to $SBIN_PATH..."
	rsync -av bin/ $SBIN_PATH
fi
