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
# Ryzom build tools ribs
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
		ribs_script "build/ryzom_build_tools.sh" "Build Tools" "RyzomTools" "NelTools" "Tools"
		end_prepare
	fi

	if ribs_prepare $1
	then
		cd $RYZOMCORE_PATH
		#check_repository $RYZOMCORE_PATH UpdateClientRepo
		end_prepare
	fi

	#Update repositories
	if [ "${OPTIONS[UpdateClientRepo]}" == "1" ]
	then
		cd $RYZOMCORE_PATH
		hg pull
		hg -v update
	fi

	RyzomTools=OFF
	Tools=OFF
	NelTools=OFF

	if [ "${OPTIONS[RyzomTools]}" == "1" ]
	then
		RyzomTools=ON
	fi

	if [ "${OPTIONS[Tools]}" == "1" ]
	then
		Tools=ON
	fi

	if [ "${OPTIONS[NelTools]}" == "1" ]
	then
		NelTools=ON
	fi

	CMAKEFLAGS="-DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CXX -DFINAL_VERSION=$FINAL_VERSION -DWITH_RYZOM_SERVER=OFF -DWITH_RYZOM_TOOLS=$RyzomTools -DWITH_RYZOM_CLIENT=OFF -DWITH_RYZOM_INSTALLER=OFF -DWITH_RYZOM_PATCH=OFF -DWITH_NEL_TESTS=OFF -DWITH_NEL_TOOLS=$NelTools -DWITH_TOOLS=$Tools -DWITH_NEL_SAMPLES=OFF -DWITH_ASSIMP=ON -DWITH_WARNINGS=OFF -DWITH_LUA53=ON -DWITH_LUA51=OFF -DWITH_LIBOVR=OFF -DWITH_QT5=OFF -DWITH_RYZOM_STEAM=OFF -DWITH_INSTALL_LIBRARIES=OFF"
	CMAKEFLAGS="$CMAKEFLAGS -DOpenGL_GL_PREFERENCE=LEGACY -DQTDIR=/usr/local/Qt-$QTVERSION -DWITH_STATIC_LIBXML2=ON -DWITH_STATIC=ON -DWITH_STATIC_DRIVERS=ON -DWITH_STATIC_EXTERNAL=ON -DWITH_UNIX_STRUCTURE=OFF -DWITH_PCH=OFF"

	RYZOMCORE_CODE_PATH=$RYZOMCORE_PATH

	LABEL="Ryzom Tools"
	DIR=$BUILD_PATH"/tools/"
	echo "--- $DIR"
	mkdir -p $DIR
	cd $DIR

	p_ok "Configuring $LABEL..."
	cmake $RYZOMCORE_CODE_PATH $CMAKEFLAGS -DCMAKE_INSTALL_PREFIX=$INSTALLDIR
	make -j$PROC

	cp $DIR/bin/* ~/bin/
fi
