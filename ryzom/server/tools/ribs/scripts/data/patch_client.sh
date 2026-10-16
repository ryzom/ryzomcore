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
# Ryzom patch clients
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
	ribs_script "data/patch_client.sh" "Patch Client" $SHARD_LIVE_DOMAINS
	exit
fi

if ribs_prepare $1
then
	have_live_domain=0
	for live_domain in $SHARD_LIVE_DOMAINS
	do
		if [[ "${OPTIONS[$live_domain]}" == "1" ]]
		then
			echo "[orange1][$live_domain] active[/orange1]"
			have_live_domain=1
		fi
	done

	if [[ "$have_live_domain" == "1" ]]
	then
		echo "Cheking changes into REF ($PATCH_PATH/ref_live) and LATESTS BNPs ($BUILD_PATH/packed)"
	fi

	cd $PATCH_PATH/patch_live
	VER=`ls -d ????? | sort -n |  tail -1`
	VERSION=`expr $VER + 0`
	p_info "Last patch have version [cyan]$VERSION[/cyan]"
	exit
fi

p_title "Patching in $PATCH_PATH"

send_chat DATA_BUILD "" "*Client Datas*: Patching..." ":inbox_tray:" "#FFC200"

cd $PATCH_PATH/patch_live
VER=`ls -d ????? | sort -n |  tail -1`
VERSION=`expr $VER + 0`
NEXT_VERSION=`expr $VER + 1`

cd $PATCH_PATH
cp live.hist history/live/$VERSION

p_info "Last patch have version $VERSION"


PACKEDDIR=$BUILD_PATH/packed
UNPACKEDDIR=$BUILD_PATH/unpacked

p_action "⚙️ Prepare patch"
cat $RYZOMDATA_PATH/tools/patch_gen/ryzom_xml_common.txt > ryzom.xml
echo "	<_BnpDirectory type=\"STRING\" value=\"$PACKEDDIR\"/>" >>  ryzom.xml
cat $RYZOMDATA_PATH/tools/patch_gen/ryzom_xml_end.txt >> ryzom.xml

p_action "⚙️ Generate patch"
sleep 1
patch_gen updateProduct ryzom.xml

PREV_VERSION=$VERSION
cd patch_live
VER=`ls -d ????? | sort -n |  tail -1`
VERSION=`expr $VER + 0`
PAD_VERSION=$(printf "%05d" $VERSION)

p_info "New patch have version $VERSION"

if [ ! -f "$VER/ryzom_$VER.idx" ]
then
	p_error "!!!!! PATCH FAILED !!!!!"
	rm -rf $VER
	# rollback latest live.hist version
	cp ../history/live/$PREV_VERSION ../live.hist
	echo "(>fail)"
	exit
fi
cd ..
echo $VERSION $VERSION > patch_live/$SHARD_DOMAIN.version
echo $VERSION `date` >> history/$SHARD_DOMAIN.ver_hist


p_title "Generating mda checksums for Ryztart"

for a in fonts packedsheets cfg exedll_win32 exedll_win64 exedll_linux64 exedll_osx
do
	cd $UNPACKEDDIR
	if [[ ! -d $a ]]
	then
		p_info "Unpacking $PACKEDDIR/$a.bnp..."
		bnp_make -u $PACKEDDIR/$a.bnp
	fi

	cd $a
	rm -f *.log
	sha1sum * > $PATCH_PATH/patch_live/$PAD_VERSION/$a.sha1

	if [[ $a == "exedll_osx" ]]
	then
		cd $BUILD_PATH/client/osx_static/Ryzom.app/Contents/Resources/
		sha1sum ../MacOS/* >> $PATCH_PATH/patch_live/$PAD_VERSION/$a.sha1
		cd $UNPACKEDDIR
	fi

	p_ok $a
done


p_title "Writing client version"

file="$BUILD_PATH/client/linux64_fv/bin/ryzom_client"
echo $file
if [ -f $file ]
then
	get_client_version $file
	get_client_version $file > $PATCH_PATH/patch_live/client.version
fi


p_title "Adding patch version to Releasenotes"

cd $PATCH_PATH
# curl -s "https://app.ryzom.com/app_releasenotes/index.php?update_version=$RELEASENOTE_TOKEN&version=$VERSION&domain=$SHARD_DOMAIN" -o/tmp/out
send_chat DATA_BUILD "" "*Client Datas*: Patch $VERSION available!"  ":inbox_tray:" "#00FF00"

p_action "Added for $SHARD_DOMAIN"

for live_domain in $SHARD_LIVE_DOMAINS
do
	SYNC=0
	if [[ "${OPTIONS[$live_domain]}" == "1" ]]
	then
		echo $VERSION $VERSION > patch_live/$live_domain.version
		echo $VERSION `date` >> history/$live_domain.ver_hist
		curl -s "https://app.ryzom.com/app_releasenotes/index.php?update_version=$RELEASENOTE_TOKEN&version=$VERSION&domain=$live_domain" -o/tmp/out
		p_action "Added for $live_domain"
		SYNC=1
	fi

	if [[ "$SYNC" == "1" ]]
	then
		p_title "Rsyncing to webserver"
		p_action "Added for $SHARD_DOMAIN"
		rm -rf ~/patchs/patch_live/olds/
		rm -f ~/patchs/patch_live/log.log
		rsync -auv --delete ~/patchs/patch_live/* app@app.ryzom.com:~/www/html/patch_live/
		
		p_warning "DON'T FORGET STEAM !!!! DO IT NOW !!!!"
		p_warning "DON'T FORGET STEAM !!!! DO IT NOW !!!!"
		p_warning "DON'T FORGET STEAM !!!! DO IT NOW !!!!"
		p_warning "DON'T FORGET STEAM !!!! DO IT NOW !!!!"
		p_warning "DON'T FORGET STEAM !!!! DO IT NOW !!!!"
	fi
done





