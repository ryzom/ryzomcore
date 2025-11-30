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
# Ryzom check patch clients
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
	ribs_script "data/check_patch_client.sh" "Check Patch Client"
	end_prepare
fi

if ribs_prepare $1
then
	echo "Nothing to prepare"
	end_prepare
fi

	echo "Cheking changes into REF ($PATCH_PATH/ref_live) and LATESTS BNPs ($BUILD_PATH/packed)"
	python3 $RYZOMDATA_PATH/tools/patch_gen/check_ref_live.py $PATCH_PATH $BUILD_PATH/packed


exit

PACKEDDIR=$BUILD_PATH/packed
UNPACKEDDIR=$BUILD_PATH/unpacked

send_chat DATA_BUILD "" "*Client Datas*: Patching..." ":inbox_tray:" "#FFC200"

clr_brown "Patching in $PATCH_PATH ..."
cd $PATCH_PATH

cd patch_live
VER=`ls -d ????? | sort -n |  tail -1`
VERSION=`expr $VER + 0`
NEXT_VERSION=`expr $VER + 1`
cd ..
cp live.hist history/live/$VERSION

cat $RYZOMDATA_PATH/tools/patch_gen/ryzom_xml_common.txt > ryzom.xml
echo "	<_BnpDirectory type=\"STRING\" value=\"$PACKEDDIR\"/>" >>  ryzom.xml
cat $RYZOMDATA_PATH/tools/patch_gen/ryzom_xml_end.txt >> ryzom.xml

patch_gen updateProduct ryzom.xml

PREV_VERSION=$VERSION
cd patch_live
VER=`ls -d ????? | sort -n |  tail -1`
VERSION=`expr $VER + 0`

if [ ! -f "$VER/ryzom_$VER.idx" ]
then
	clr_red "!!!!! PATCH FAILED !!!!!"
	rm -rf $VER
	# rollback latest live.hist version
	cp ../history/live/$PREV_VERSION ../live.hist
	echo "(>fail)"
	exit
fi
cd ..
echo $VERSION $VERSION > $SHARD_DOMAIN.version


echo $VERSION `date` >> history/$SHARD_DOMAIN.ver_hist

curl -s "https://app.ryzom.com/app_releasenotes/index.php?update_version=x8dslpOKksdoe9sicd293dokdoeo&version=$VERSION&domain=$SHARD_DOMAIN"
send_chat DATA_BUILD "" "*Client Datas*: Patch $VERSION available!"  ":inbox_tray:" "#00FF00"

## TODO : change it to a better way
if [ ! -z "$SHARD_DOMAIN2"]
then
	cd patch_live
	echo $VERSION $VERSION > $SHARD_DOMAIN2.version
	cd ..
	echo $VERSION `date` >> history/$SHARD_DOMAIN2.ver_hist
	curl -s "https://app.ryzom.com/app_releasenotes/index.php?update_version=x8dslpOKksdoe9sicd293dokdoeo&version=$VERSION&domain=$SHARD_DOMAIN2"
fi


