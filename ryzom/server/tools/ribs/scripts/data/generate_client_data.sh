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
# Ryzom generate client data
#

. $RIBS_PATH/utils.sh
. $RIBS_PATH/ryzom_utils.sh
. $RIBS_PATH/ryzom_prefs.sh
. $RIBS_PATH/membash.sh

if [[ ! "$RIBS_GROUPS" == *":dev:"* ]]
then
	exit
fi

if [[ -z "$*" ]]
then
	ribs_script "data/generate_client_data.sh" "Generate Client Data" "+Sheets" "+Data" "+Clients"
	exit
fi

MODE="fv"

cd $RYZOMCORE_PATH/ryzom/client/src
REVISION=$(git rev-list HEAD --count .)
cd $RYZOMCORE_PATH/

BRANCH=$(git rev-parse --abbrev-ref HEAD)
if [ "$BRANCH" == "main/yubo-dev" ]
then
	DOMAIN="Alpha /"
elif [ "$BRANCH" == "main/gingo-test" ]
then
	DOMAIN="Beta /"
else
	DOMAIN="Omega /"
fi

p_info "Domain : $DOMAIN"

function compare_versions {
	if [[ ! "$2" == "$3" ]]
	then
		p_warning " $1 with $(p_badvalue "v$3")"
	else
		p_ok " $1 with $(p_value "v$3")"
	fi
}


if ribs_prepare $1
then
	p_title "Cheking repositories"

	if [[ "${OPTIONS[Data]}" == "1" ]] || [[ "${OPTIONS[Sheets]}" == "1" ]]
	then
		check_git_repository $RYZOMDATA_PATH UpdateDataRepo
		check_git_repository $RYZOMSERVERDATA_PATH UpdateServerRepo
	fi

	if [[ "${OPTIONS[Clients]}" == "1" ]]
	then
		check_git_repository $RYZOMCORE_PATH UpdateClientRepo

		################################################################
		# We get binaries and required files from builds
		################################################################
		
		p_title "Checking client versions"
		# Getting versions
		VERSION=$REVISION
		p_ok "REVISION = [yellow]v$REVISION[/yellow]"

		file="$BUILD_PATH/client/linux64_$MODE/bin/ryzom_client"
		if [ -f $file ]
		then
			cp $file ${file}_dbg
			strip $file
			LINUX64_VERSION=$(get_client_version $file)
			compare_versions "Linux 64 Client \[$MODE]" "$VERSION" "$LINUX64_VERSION" "$file"
		else
			compare_versions "Linux 64 Client \[$MODE]" "$VERSION" " MISSING" "$file"
		fi

		file="$BUILD_PATH/client/win32_$MODE/bin/ryzom_client_r.exe"
		if [ -f $file ]
		then
			WIN32_VERSION=$(get_client_version $file)
			compare_versions "Windows 32 Client \[$MODE]" "$VERSION" "$WIN32_VERSION" "$file"
		else
			compare_versions "Windows 32 Client \[$MODE]" "$VERSION" " MISSING" "$file"
		fi

		file="$BUILD_PATH/client/win64_$MODE/bin/ryzom_client_r.exe"
		if [ -f $file ]
		then
			WIN64_VERSION=$(get_client_version $file)
			compare_versions "Windows 64 Client \[$MODE]" "$VERSION" "$WIN64_VERSION" "$file"
		else
			compare_versions "Windows 64 Client \[$MODE]" "$VERSION" " MISSING" "$file"
		fi

		# Rename osx packages
		file=$(ls -tp $BUILD_PATH/client/ryzom_static_osx_*.7z 2> /dev/null | head -n 1)
		if [[ ! -z $file ]] && [[ -f $file ]]
		then
			cp $file $BUILD_PATH/client/ryzom_static_osx.7z
		fi

		file=$(ls -tp $BUILD_PATH/client/ryzom_steam_osx_*.7z 2> /dev/null | head -n 1)
		if [[ ! -z $file ]] && [[ -f $file ]]
		then
			cp $file $BUILD_PATH/client/ryzom_steam_osx.7z
		fi

		file=$(ls -tp $BUILD_PATH/client/ryzom_installer_osx_*.7z 2> /dev/null | head -n 1)
		if [[ ! -z $file ]] && [[ -f $file ]]
		then
			cp $file $BUILD_PATH/client/ryzom_installer_osx.7z
		fi

		rm -rf $BUILD_PATH/client/osx_static/
		mkdir $BUILD_PATH/client/osx_static/
		if [ -f $BUILD_PATH/client/ryzom_static_osx.7z ]
		then
			7z x -y -o$BUILD_PATH/client/osx_static $BUILD_PATH/client/ryzom_static_osx.7z > /dev/null
			if [ -d $BUILD_PATH/builds/client/osx_static/*/bin/Ryzom.app/ ]
			then
				mv $BUILD_PATH/builds/client/osx_static/*/bin/Ryzom.app/ $BUILD_PATH/builds/client/osx_static/
			fi
		fi

		file="$BUILD_PATH/client/osx_static/Ryzom.app/Contents/MacOS/Ryzom "
		if [ -f $file ]
		then
			OSX_VERSION=$(get_client_version $file)
			compare_versions "OsX 64 Client" "$VERSION" "$OSX_VERSION" "$file"
		else
			compare_versions "OsX 64 Client" "$VERSION" " MISSING" "$file"
		fi
		echo " "

		file="$BUILD_PATH/client/linux64_steam/bin/ryzom_client"
		if [ -f $file ]
		then
			strip $file
			LINUX64_VERSION=$(get_client_version $file)
			compare_versions "Linux 64 Client \[Steam]" "$VERSION" "$LINUX64_VERSION" "$file"
		else
			compare_versions "Linux 64 Client \[Steam]" "$VERSION" " MISSING" "$file"
		fi

		file="$BUILD_PATH/client/win64_steam/bin/ryzom_client_r.exe"
		if [ -f $file ]
		then
			WIN64_VERSION=$(get_client_version $file)
			compare_versions "Windows 64 Client \[Steam]" "$VERSION" "$WIN64_VERSION" "$file"
		else
			compare_versions "Windows 64 Client \[Steam]" "$VERSION" " MISSING" "$file"
		fi

		if [ -f $BUILD_PATH/client/ryzom_steam_osx.7z ]
		then
			mkdir -p $BUILD_PATH/client/osx_steam
			rm -rf $BUILD_PATH/client/osx_steam/*
			7z x -y -o$BUILD_PATH/client/osx_steam $BUILD_PATH/client/ryzom_steam_osx.7z > /dev/null
		fi


		file="$BUILD_PATH/client/osx_steam/Ryzom.app/Contents/MacOS/Ryzom"
		if [ -f $file ]
		then
			OSX_VERSION=$(get_client_version $file)
			compare_versions "OsX 64 Client \[Steam]" "$VERSION" "$OSX_VERSION" "$file"
		else
			compare_versions "OsX 64 Client \[Steam]" "$VERSION" " MISSING" "$file"
		fi
	fi

	p_title "Checking patch version"

	MAX_VERSION=$(cat $PATCH_PATH/patch_live/ryzom.version | cut -d" " -f1)
	p_info "Using ref bnps until [yellow]v$MAX_VERSION[/yellow]"

	end_prepare
fi

#Update repositories
if [ "${OPTIONS[UpdateDataRepo]}" == "1" ]
then
	update_git_repository $RYZOMDATA_PATH
fi

if [ "${OPTIONS[UpdateServerRepo]}" == "1" ]
then
	update_git_repository $RYZOMSERVERDATA_PATH
fi

send_chat DATA_BUILD "" "*Client Data*: Generating..." ":package:" "#FFC200"

PACKEDDIR=$BUILD_PATH/packed
PACKEDMD5DIR=$BUILD_PATH/packed_md5
PACKEDTMPDIR=/tmp/packed
UNPACKEDTMPDIR=/tmp/unpacked
UNPACKEDDIR=$BUILD_PATH/unpacked
RIBS_I=/run/user/$UID/ribs_i

mkdir -p $PACKEDDIR
mkdir -p $PACKEDMD5DIR
rm -rf $PACKEDTMPDIR && mkdir -p $PACKEDTMPDIR
rm -rf $UNPACKEDDIR && mkdir -p $UNPACKEDDIR
rm -rf $UNPACKEDTMPDIR && mkdir -p $UNPACKEDTMPDIR
mkdir $UNPACKEDTMPDIR/origin
mkdir $UNPACKEDTMPDIR/current

MAX_VERSION=$(cat $PATCH_PATH/patch_live/ryzom.version | cut -d" " -f1)
p_info "Use ref bnps until v$MAX_VERSION"
DATADIRS=$(python3 $RYZOMDATA_PATH/tools/patch_gen/get_latest_bnps.py $PATCH_PATH/ref_live $MAX_VERSION)

function multimd5 {
	touch $PACKEDTMPDIR/$2.md5
	md5sum "$1" > "$PACKEDMD5DIR/$2.md5"
	rm $PACKEDTMPDIR/$2.md5
	print_percent $(mc_incr RIBS_I 1) $total_files
}

function unbnp {
	touch $PACKEDTMPDIR/$2.bnp
	bnp_make -u $1 -o $2
	p_action "📤 Unbnping $package $(p_value v$version)"
	rm $PACKEDTMPDIR/$2.bnp
}

if [ "${OPTIONS[Data]}" == "1" ]
then

	p_title "Copying final BNP/E into $PACKEDDIR. Unbnping refs into $UNPACKEDDIR"

	total_files=$(echo "$DATADIRS" | wc -w)
	mc_set RIBS_I 0 0

	cd "$UNPACKEDDIR"
	for DATADIR in $DATADIRS
	do
		filename=$(basename $DATADIR)
		bnpfile=$(echo $filename | cut -d'.' -f1)
		extension=$(echo $filename | cut -d'.' -f2)
		package=$(echo $bnpfile | rev | cut -d'_' -f2- | rev)
		version=$(echo $bnpfile | rev | cut -d'_' -f1  | rev)

		while [ $(ls $PACKEDTMPDIR | wc -w) -gt 4 ]
		do
			sleep 0.1
		done
		
		if [[ -d "$RYZOMDATA_PATH/final_bnps/$package" ]]
		then
			# FOLDERS in final_bnps. So we need extract ref bnp into $UNPACKEDDIR
			unbnp "$DATADIR" "$package" &
		elif [[ -f "$RYZOMDATA_PATH/final_bnps/$package.bnp" ]]
		then
			# BNP in final_bnps. So we copy and forget ref_live
			if [ ! -f "$PACKEDDIR/$package.bnp" ] || [ "$RYZOMDATA_PATH/final_bnps/$package.bnp" -nt "$PACKEDDIR/$package.bnp" ]
			then
				p_action "🌀 Copying $package.bnp"
				cp -p "$RYZOMDATA_PATH/final_bnps/$package.bnp" "$PACKEDDIR/$package.bnp"
			fi
		elif [[ -f "$RYZOMDATA_PATH/final_bnps/events/bnps/$package.bnpe" ]]
		then
			# BNPE in final_bnps. So we copy and forget ref_live
			if [ ! -f "$PACKEDDIR/$package.bnpe" ] || [ "$RYZOMDATA_PATH/final_bnps/events/bnps/$package.bnpe" -nt "$PACKEDDIR/$package.bnpe" ]
			then
				p_action "🌀 Copying $package.bnpe"
				cp -p "$RYZOMDATA_PATH/final_bnps/events/bnps/$package.bnpe" "$PACKEDDIR/$package.bnpe"
			fi
		else
			# Simply copy the file if need to PACKEDDIR
			if [ ! -f "$PACKEDDIR/$package.$extension" ] || [ "$DATADIR" -nt "$PACKEDDIR/$package.$extension" ]
			then
				p_action "🌀 Copying $package.$extension $(p_value v$version)..."
				cp -p "$DATADIR" "$PACKEDDIR/$package.$extension"
			fi
		fi

		if [ ! -f "$PACKEDMD5DIR/$package.md5" ] || [ "$DATADIR" -nt "$PACKEDMD5DIR/$package.md5" ]
		then
			multimd5 "$DATADIR" "$package" I &
		else
			print_percent $(mc_incr RIBS_I 1) $total_files
		fi
	done

	p_title "Copying Datas from Repository"
	TOTAL=3

	D=$UNPACKEDDIR/gamedev
	mkdir -p $D
	TRANSLATEDDIR=$RYZOMDATA_PATH/translations/translated
	p_action "🌀 Copying *ALL* Translations from $TRANSLATEDDIR..."

	cp -p $TRANSLATEDDIR/classificationtype_words_*.txt $D
	#cp -p $TRANSLATEDDIR/creature_words_*.txt $D
	cp -p $TRANSLATEDDIR/faction_words_*.txt $D
	#cp -p $TRANSLATEDDIR/item_words_*.txt $D
	cp -p $TRANSLATEDDIR/outpost_words_*.txt $D
	#cp -p $TRANSLATEDDIR/place_words_*.txt $D
	#cp -p $TRANSLATEDDIR/sbrick_words_*.txt $D
	cp -p $TRANSLATEDDIR/skill_words_*.txt $D
	cp -p $TRANSLATEDDIR/sphrase_words_*.txt $D
	#cp -p $TRANSLATEDDIR/title_words_*.txt $D
	#cp -p $TRANSLATEDDIR/bot_names.txt $D
	rm -f $D/wk.uxt $D/*_wk.txt

	if [ "$SHARD_DOMAIN" == "ryzom_dev" ]
	then
		SUFFIX="_wk"
	else
		SUFFIX=""
	fi
	
	for generator in $($SCRIPT_DIR/generators/*.sh 2> /dev/null)
	do
		bash $generator $D $SUFFIX $TRANSLATEDDIR
	done
	print_percent 1 $TOTAL

	p_action "🌀 Copying Leveldesign and sheet_id.bin from $RYZOMSERVERDATA_PATH to $RYZOMDATA_PATH/final_bnps/leveldesign..."
	D=$RYZOMDATA_PATH/final_bnps/leveldesign
	mkdir -p $D
	S=$RYZOMSERVERDATA_PATH
	C=$RYZOMDATA_PATH/leveldesign
	cp -v $S/game_element/sheet_id.bin $D
	cp -v $S/game_element/sheets.txt $D
	cp $C/DFN/game_elem/_creature/_creature_3d_eyes_color.typ $D
	cp $C/DFN/game_elem/_creature/_creature_3d_hair_color.typ $D
	cp $S/game_element/anim/mode2animset.string_array $D
	cp $C/DFN/game_elem/_anim/string_array.dfn $D
	cp $C/DFN/basics/string.typ $D
	cp $C/world/static_fame.txt $D
	print_percent 2 $TOTAL

	p_action "⚗️  Building Interface from $RYZOMDATA_PATH/assets_src/interfaces and copying..."
	D=$UNPACKEDDIR/interfaces
	mkdir -p $D

	I=$RYZOMDATA_PATH/final_bnps/interfaces
	O=$RYZOMDATA_PATH/assets_src/interfaces

	rm -f $I/texture_interfaces_v3_*
	rm -f $I/texture_interfaces_dxtc_*
	build_interface $I/texture_interfaces_v3_2x.png $O/texture_interfaces_v3_2x
	build_interface $I/texture_interfaces_dxtc_2x.png $O/texture_interfaces_dxtc_2x

	build_interface $I/texture_interfaces_v3_login.png $O/texture_interfaces_v3_login
	build_interface $I/texture_interfaces_v3_outgame_ui.png $O/texture_interfaces_v3_outgame_ui

	rm -f $I/new_texture_interfaces_dxtc.*
	rm -f $I/texture_extra.*

	rm -f $I/a0[1-3]_[1-9]* $I/h0[1-3]_[1-9]* $I/n0[1-3]_[1-9]* $I/o0[1-3]_[1-9]*

	################################################################
	# If image have dds and png version. We delete the png
	################################################################
	for png in $I/*.png
	do
		file=$(basename "$png" .png)
		if [ -f "$file.dds" ]
		then
			rm "$png"
		fi
	done
	cp -a $I/* $D

	print_percent 3 $TOTAL
	p_title "Copying Bnp Changes in repository from $RYZOMDATA_PATH/final_bnps..."
	cd $RYZOMDATA_PATH/final_bnps/
	DATADIRS=$(ls -1)
	cd $CURDIR
	NUMDATAS=$(echo "$DATADIRS" | wc -w)
	I=0
	
	for DATADIR in $DATADIRS
	do
		I=$(($I + 1))
		print_percent $I $NUMDATAS

		if [ "$DATADIR" == "events" ] || [ "$DATADIR" == "packedsheets" ] || [ "$DATADIR" == "data_common" ] || [ "$DATADIR" == "gamedev" ] || [ "$DATADIR" == "fonts" ]  || [ "$DATADIR" == "cfg" ] 
		then
			continue
		fi

		DATABNPDIR=$RYZOMDATA_PATH/final_bnps/$DATADIR
		UNPACKEDBNP=$UNPACKEDDIR/$DATADIR

		if [ -d $UNPACKEDBNP ]
		then
			subfolders=$(find $DATABNPDIR/* -type d)
			if [ ! -z "$subfolders" ]
			then
				mkdir $UNPACKEDTMPDIR/$DATADIR
				find $DATABNPDIR -type f -exec cp '{}' $UNPACKEDTMPDIR/$DATADIR/ \;
				RSYNC_CHANGES=$(rsync -ic $UNPACKEDTMPDIR/$DATADIR/* $UNPACKEDBNP/)
			else
				RSYNC_CHANGES=$(rsync -ic $DATABNPDIR/* $UNPACKEDBNP/)
			fi

			if [ -z "$RSYNC_CHANGES" ]
			then
				touch $UNPACKEDBNP/skip
			else
				p_action "💈 $DATADIR..."
			fi
		fi
	done

	p_title "Copying Data Common from $RYZOMCORE_PATH/ryzom/common/data_common..."
	TOTAL=3
	D=$UNPACKEDDIR/data_common
	mkdir -p $D
	for file in $(find $RYZOMCORE_PATH/ryzom/common/data_common/ -type f)
	do
		cp $file $D
	done
	rm -f "$D/moved to ryzom_core"

	p_action "🌀 Copying Cfg from $RYZOMCORE_PATH/ryzom/client/cfg..."
	
	D=$UNPACKEDDIR/cfg
	mkdir -p $D
	for file in $(find $RYZOMCORE_PATH/ryzom/client/cfg/ -type f)
	do
		cp $file $D
	done
	rm -f "$D/moved to ryzom_core"
	print_percent 1 $TOTAL

	p_action "🌀 Copying Gamedev from $RYZOMCORE_PATH/ryzom/client/data/gamedev..."
	D=$UNPACKEDDIR/gamedev
	mkdir -p $D
	for file in $(find $RYZOMCORE_PATH/ryzom/client/data/gamedev -type f)
	do
		cp $file $D
	done
	rm -f "$D/moved to ryzom_core"
	print_percent 2 $TOTAL

	p_action "🌀 Copying Fonts from $RYZOMCORE_PATH/ryzom/client/data/fonts..."
	D=$UNPACKEDDIR/fonts
	mkdir -p $D
	for file in $(find $RYZOMCORE_PATH/ryzom/client/data/fonts -type f)
	do
		cp $file $D
	done
	rm -f "$D/moved to ryzom_core"

	# real files are in jungle_maps.bnp
	rm -f $UNPACKEDDIR/objects/ju_bamboo_tronc_au.dds

	# files are in packedsheets
	rm -f $UNPACKEDDIR/data_common/visual_slot.tab
	rm -f $UNPACKEDDIR/data_common/vs_index.txt

	# unused
	rm -rf $UNPACKEDDIR/examples
	rm -rf $UNPACKEDDIR/events

	rm -f  $UNPACKEDDIR/interfaces/newbieland_city.png
	print_percent 3 $TOTAL
fi

if [ "${OPTIONS[Sheets]}" == "1" ]
then
	p_title "Building Packed Sheets"
	D=$BUILD_PATH/packedsheets
	rm -rf $D
	mkdir $D

	cd $D
	cat > sheets_packer.cfg <<- EOF
	DataPath = { "$RYZOMDATA_PATH/leveldesign", "$RYZOMSERVERDATA_PATH/game_element", "$RYZOMSERVERDATA_PATH/primitives", "$RYZOMDATA_PATH/leveldesign/world", "$D" };
	WorldSheet = "ryzom.world";
	PrimitivesPath = "$RYZOMSERVERDATA_PATH/primitives";
	OutputDataPath = "$D";
	LigoPrimitiveClass = "world_editor_classes.xml";
	DumpVisualSlotsIndex = 1;
	EOF

	sheets_packer | python3 $RIBS_PATH/filter_output.py

	cp visual_slot.tab $RYZOMDATA_PATH/common/
	cp vs_index.txt $RYZOMDATA_PATH/common/

	# Apply new packedsheets
	D=$UNPACKEDDIR/packedsheets
	rm -rf $D
	mkdir $D
	cp $BUILD_PATH/packedsheets/* $D
	echo "FAKE FAKE FAKE FAKE" > $D/fake.txt
fi


#send_chat DATA_BUILD "" "*Client Data*: Done!" ":package:" "#00FF00"

if [ "${OPTIONS[Clients]}" == "1" ]
then
	p_title "Preparing ExeDLL bnps"
	TOTAL=6
	mkdir -p $UNPACKEDDIR/exedll/
	mkdir -p $UNPACKEDDIR/exedll_win32/
	mkdir -p $UNPACKEDDIR/exedll_linux32/

	rm -rf $UNPACKEDDIR/exedll_linux64/
	rm -rf $UNPACKEDDIR/exedll_win64/
	rm -rf $UNPACKEDDIR/exedll_osx/
	mkdir $UNPACKEDDIR/exedll_linux64/
	mkdir $UNPACKEDDIR/exedll_win64/
	mkdir $UNPACKEDDIR/exedll_osx/

	if [ "${OPTIONS[Data]}" != "1" ]
	then
		for DATADIR in $DATADIRS
		do
			bnpfile=$(basename $DATADIR .bnp)
			bnpfolder=${bnpfile::-6}
			if [[ $bnpfolder == "exedll_"* ]]
			then
				md5sum $DATADIR > $PACKEDMD5DIR/$bnpfolder.md5
			fi
		done
	fi

	p_action "💠 Copying Ryzom Client files in exedll_win32..."
	cp -a $RYZOMEXTERNALS_PATH/fv/win32/* $UNPACKEDDIR/exedll_win32/
	#cp -a $BUILD_PATH/client/win32_fv/bin/*.dll $UNPACKEDDIR/exedll_win32/
	cp -a $BUILD_PATH/client/win32_fv/bin/ryzom_client_patcher.exe $UNPACKEDDIR/exedll_win32/
	cp -a $BUILD_PATH/client/win32_fv/bin/ryzom_client_r.exe $UNPACKEDDIR/exedll_win32/
	#cp -a $BUILD_PATH/client/win32_fv/bin/ryzom_configuration_qt_r.exe $UNPACKEDDIR/exedll_win32/
	cp -a $RYZOMCORE_PATH/ryzom/client/client_default.cfg $UNPACKEDDIR/exedll_win32/
	cp -a $RYZOMCORE_PATH/ryzom/client/windows/* $UNPACKEDDIR/exedll_win32/
	print_percent 1 $TOTAL

	p_action "💠 Copying Ryzom Client files in exedll_win64..."
	cp -a $RYZOMEXTERNALS_PATH/fv/win64/* $UNPACKEDDIR/exedll_win64/
	#cp -a $BUILD_PATH/client/win64_fv/bin/*.dll $UNPACKEDDIR/exedll_win64/
	cp -a $BUILD_PATH/client/win64_fv/bin/ryzom_client_patcher.exe $UNPACKEDDIR/exedll_win64/
	cp -a $BUILD_PATH/client/win64_fv/bin/ryzom_client_r.exe $UNPACKEDDIR/exedll_win64/
	#cp -a $BUILD_PATH/client/win64_fv/bin/ryzom_configuration_qt_r.exe $UNPACKEDDIR/exedll_win64/
	cp -a $RYZOMCORE_PATH/ryzom/client/client_default.cfg $UNPACKEDDIR/exedll_win64/
	cp -a $RYZOMCORE_PATH/ryzom/client/windows/* $UNPACKEDDIR/exedll_win64/
	print_percent 2 $TOTAL

	p_action "🐧 Copying Ryzom Client files in exedll_linux64..."
	cp -a $RYZOMEXTERNALS_PATH/fv/linux64/* $UNPACKEDDIR/exedll_linux64/
	cp -a $BUILD_PATH/client/linux64_fv/bin/ryzom_client $UNPACKEDDIR/exedll_linux64/
	cp -a $BUILD_PATH/client/linux64_fv/bin/ryzom_client_patcher $UNPACKEDDIR/exedll_linux64/
	#cp -a $BUILD_PATH/client/linux64_fv/bin/ryzom_configuration_qt $UNPACKEDDIR/exedll_linux64/
	cp -a $RYZOMCORE_PATH/ryzom/client/client_default.cfg $UNPACKEDDIR/exedll_linux64/
	cp -a $RYZOMCORE_PATH/ryzom/client/unix/upgd_nl.sh $UNPACKEDDIR/exedll_linux64/
	print_percent 3 $TOTAL

	p_action "🍎 Copying Ryzom Client files in Ryzom.app (osx)..."
	mkdir -p $BUILD_PATH/client/osx_static
	cd $BUILD_PATH/client/osx_static
	cp -a $RYZOMCORE_PATH/ryzom/client/client_default.cfg "Ryzom.app/Contents/Resources/"
	zip -qr $UNPACKEDDIR/exedll_osx/Ryzom.zip "Ryzom.app"
	print_percent 4 $TOTAL

	p_action "🍎 Copying Ryzom Cfg files in Ryzom.app (osx steam)..."
	mkdir -p $BUILD_PATH/client/osx_steam
	cd $BUILD_PATH/client/osx_steam
	cp -a $RYZOMCORE_PATH/ryzom/client/client_default.cfg "Ryzom.app/Contents/Resources/"
	print_percent 5 $TOTAL

	cp -a $RYZOMCORE_PATH/ryzom/client/client_default.cfg $UNPACKEDDIR/exedll_osx/
	cp -a $RYZOMCORE_PATH/ryzom/client/macosx/upgd_nl.sh $UNPACKEDDIR/exedll_osx/
	print_percent 6 $TOTAL


	p_title "Preparing Steam Depots" "https://upload.wikimedia.org/wikipedia/commons/a/ae/Steam_logo.svg"
	TOTAL=3
	STEAM_DEPOT_PATH=$BUILD_PATH/packages/steam

	p_action "💠 Copying Ryzom Client Win64..."
	rm -rf $STEAM_DEPOT_PATH/win64/
	mkdir  $STEAM_DEPOT_PATH/win64/
	cp -ar $RYZOMEXTERNALS_PATH/steam/win64/* $STEAM_DEPOT_PATH/win64/
	#cp -a $BUILD_PATH/client/win64_steam/bin/*.dll $STEAM_DEPOT_PATH/win64/
	cp -a $BUILD_PATH/client/win64_steam/bin/ryzom_client_r.exe $STEAM_DEPOT_PATH/win64/
	#cp -a $BUILD_PATH/client/win64_steam/bin/ryzom_configuration_qt_r.exe $STEAM_DEPOT_PATH/win64/
	cp -a $RYZOMCORE_PATH/ryzom/client/client_default.cfg $STEAM_DEPOT_PATH/win64/
	cp -a $RYZOMCORE_PATH/ryzom/client/windows/* $STEAM_DEPOT_PATH/win64/
	rm  -f $STEAM_DEPOT_PATH/win64/ryzom_client_patcher.exe
	print_percent 1 $TOTAL

	p_action "🐧 Copying Ryzom Client Linux64..."
	rm -rf $STEAM_DEPOT_PATH/linux64/
	mkdir $STEAM_DEPOT_PATH/linux64/
	cp -ar $RYZOMEXTERNALS_PATH/steam/linux64/* $STEAM_DEPOT_PATH/linux64/
	cp -a $BUILD_PATH/client/linux64_steam/bin/ryzom_client $STEAM_DEPOT_PATH/linux64/
	#cp -a $BUILD_PATH/client/ryzom_configurator/steam_linux64 $STEAM_DEPOT_PATH/linux64/ryzom_configuration_qt
	cp -a $RYZOMCORE_PATH/ryzom/client/client_default.cfg $STEAM_DEPOT_PATH/linux64/
	cp -a $RYZOMCORE_PATH/ryzom/client/unix/upgd_nl.sh $STEAM_DEPOT_PATH/linux64/
	cp -a $RYZOMCORE_PATH/ryzom/client/unix/ryzom_client.png $STEAM_DEPOT_PATH/linux64/steam_icon_373720.png
	print_percent 2 $TOTAL

	p_action "🍎 Copying Ryzom Client OSX..."
	mkdir -p  $STEAM_DEPOT_PATH/osx/
	rm -rf $STEAM_DEPOT_PATH/osx/Ryzom.app
	7z x -bso0 -bsp0 -y -o$STEAM_DEPOT_PATH/osx $BUILD_PATH/client/ryzom_steam_osx.7z
	cp -a $RYZOMCORE_PATH/ryzom/client/client_default.cfg $STEAM_DEPOT_PATH/osx/Ryzom.app/Contents/Resources
	rm -rf $UNPACKEDDIR/exedll
	rm -rf $UNPACKEDDIR/exedll_linux32
	print_percent 3 $TOTAL
fi

#################### packing
p_title "Packing BNPs..."

# clean files not directories (don't remove them)
rm $UNPACKEDDIR/* 2> /dev/null
DATADIRS=$(ls $UNPACKEDDIR)
NUMDATA=$(echo "$DATADIRS" | wc -w)
I=0
for f in $DATADIRS
do
	if [ ! -f "$UNPACKEDDIR/$f/skip" ]
	then
		find $UNPACKEDDIR/$f -depth -type f | while read SRC
		do
			basename=$(basename "${SRC}")
			dirname=$(dirname "${SRC}")
			goodname=$(echo "$dirname/$basename" | tr '[A-Z]' '[a-z]')
			if [ "$SRC" != "$goodname" ]
			then
				mv -T "${SRC}" "$goodname"
			fi
		done

		if [ "$f" == "interfaces" ]
		then
			find $UNPACKEDDIR/$f -mindepth 1 -maxdepth 1 -type d -exec rm -r {} \;
		fi

		bnp_make -p -o $PACKEDTMPDIR $UNPACKEDDIR/$f > /dev/null
		MD5SUM_NEW=$(md5sum $PACKEDTMPDIR/$f.bnp | cut -d " " -f 1)
		MD5SUM_ORG=""
		if [ -f "$PACKEDMD5DIR/$f.md5" ]
		then
			MD5SUM_ORG=$(cat $PACKEDMD5DIR/$f.md5 | cut -d' ' -f 1)
		fi

		if [ "$MD5SUM_NEW" != "$MD5SUM_ORG" ]
		then
			p_warning "Bnp $f are diff"
			if [ -f "$PACKEDMD5DIR/$f.md5" ]
			then
				BNP_ORG=$(cat $PACKEDMD5DIR/$f.md5 | cut -d' ' -f 3)
				bnp_make -u $BNP_ORG -o $UNPACKEDTMPDIR/origin/
				#bnp_make -u $PACKEDTMPDIR/$f.bnp -o $UNPACKEDTMPDIR/current/
				diff $UNPACKEDTMPDIR/origin/ $UNPACKEDDIR/$f/
				rm -rf $UNPACKEDTMPDIR/origin/*
			else
				echo "New BNP"
				echo "$MD5SUM_NEW" > $PACKEDMD5DIR/$f.md5
			fi
			cp $PACKEDTMPDIR/$f.bnp $PACKEDDIR/$f.bnp
		fi
	fi
	I=$(($I + 1))
	print_percent $I $NUMDATA
done

if [ -f $PACKEDDIR/_occ_version_objects.bnp ]
then
	mv $PACKEDDIR/_occ_version_objects.bnp _occ_version_objects.bnpe
fi

curl -s "https://app.ryzom.com/app_releasenotes/index.php?update_version=$RELEASENOTE_TOKEN&version=$VERSION&domain=$SHARD_DOMAIN" > /dev/null
