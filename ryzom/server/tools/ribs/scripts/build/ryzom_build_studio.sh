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
# Ryzom build studio ribs
#

. $RIBS_PATH/utils.sh
. $RIBS_PATH/ryzom_utils.sh
. $RIBS_PATH/ryzom_prefs.sh
. $RIBS_PATH/ryzom_compilation_prefs.sh

PROC=$(nproc)

if [[ "$RIBS_GROUPS" == *":dev:"* ]]
then
	if [[ -z "$*" ]]
	then
		ribs_script "build/ryzom_build_studio.sh" "Build Studio"
		end_prepare
	fi

	if ribs_prepare $1
	then
		cd $RYZOMCORE_PATH
		check_repository $RYZOMCORE_PATH UpdateClientRepo

		if [ "$USE_CHROOT" == "1" ]
		then
			p_info "Compilation will use a chrooted environement"
		else
			p_info "Compilation will use own linux environement"
		fi
		
		end_prepare
	fi

	#Update repositories
	if [ "${OPTIONS[UpdateClientRepo]}" == "1" ]
	then
		cd $RYZOMCORE_PATH
		hg pull
		hg -v update
	fi

	#CMAKEFLAGS="-DCMAKE_BUILD_TYPE=Release -DFINAL_VERSION=1 -DWITH_RYZOM_SERVER=OFF -DWITH_RYZOM_TOOLS=OFF -DWITH_RYZOM_CLIENT=OFF -DWITH_RYZOM_INSTALLER=OFF -DWITH_RYZOM_PATCH=OFF -DWITH_NEL_TESTS=OFF -DWITH_NEL_TOOLS=OFF -DWITH_TOOLS=OFF -DWITH_NEL_SAMPLES=OFF -DWITH_ASSIMP=OFF -DWITH_WARNINGS=OFF -DWITH_LUA53=ON -DWITH_LUA51=OFF -DWITH_LIBOVR=OFF -DWITH_QT5=OFF -DWITH_RYZOM_STEAM=OFF -DWITH_INSTALL_LIBRARIES=OFF"
	#CMAKEFLAGS="$CMAKEFLAGS -DOpenGL_GL_PREFERENCE=LEGACY -DQTDIR=/usr/local/Qt-$QTVERSION -DWITH_STATIC_LIBXML2=ON -DWITH_STATIC=OFF -DWITH_STATIC_DRIVERS=OFF -DWITH_STATIC_EXTERNAL=ON -DWITH_UNIX_STRUCTURE=OFF -DWITH_PCH=OFF"

	CMAKEFLAGS="-DCMAKE_BUILD_TYPE=Release -DFINAL_VERSION=1 -DWITH_RYZOM_SERVER=OFF -DWITH_RYZOM_TOOLS=OFF -DWITH_RYZOM_CLIENT=OFF -DWITH_RYZOM_INSTALLER=OFF -DWITH_RYZOM_PATCH=OFF -DWITH_NEL_TESTS=OFF -DWITH_NEL_TOOLS=OFF -DWITH_TOOLS=OFF -DWITH_NEL_SAMPLES=OFF -DWITH_ASSIMP=OFF -DWITH_WARNINGS=OFF -DWITH_LUA53=OFF -DWITH_LUA51=OFF -DWITH_LIBOVR=OFF -DWITH_RYZOM_STEAM=OFF -DWITH_INSTALL_LIBRARIES=OFF -DWITH_UNIX_STRUCTURE=OFF -DWITH_PCH=OFF -DWITH_STUDIO=ON -DWITH_QT=ON -DWITH_QT5=OFF -DOpenGL_GL_PREFERENCE=LEGACY"

	LABEL="Ryzom Studio"
	DIR=$BUILD_PATH"/studio/"
	echo "--- $DIR"
	mkdir -p $DIR
	cd $DIR

	p_ok "Configuring $LABEL..."

	if [ "$USE_CHROOT" == "1" ]
	then
		CHROOT=steam64
		SESSION_FILE="$RIBS_USER_PATH/data/build_client_chroot.session"
		CHROOT_SESSION=$(schroot --begin-session -c $CHROOT)
		echo "$CHROOT_SESSION" > "$SESSION_FILE"
		schroot --run-session -c "$CHROOT_SESSION" -- cmake $RYZOMCORE_PATH/code $CMAKEFLAGS -DCMAKE_INSTALL_PREFIX=$INSTALLDIR
	else
		cmake $RYZOMCORE_PATH/code $CMAKEFLAGS -DCMAKE_INSTALL_PREFIX=$INSTALLDIR
	fi

	check_success "$CHROOT_SESSION"
	 
	make -j$PROC
	check_success

	cp $DIR/bin/* ~/bin/
fi
