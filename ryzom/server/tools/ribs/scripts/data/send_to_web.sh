#!/bin/bash

. $RIBS_PATH/utils.sh
. $RIBS_PATH/ryzom_utils.sh
. $RIBS_PATH/ryzom_prefs.sh

if [[ ! "$RIBS_GROUPS" == *":dev:"* ]]
then
	exit
fi

if [[ -z "$*" ]]
then
	ribs_script "data/send_to_web.sh" "Send to Web"
	end_prepare
fi

if ribs_prepare $1
then
	echo "Nothing to prepare"
	end_prepare
fi


PACKEDDIR=$BUILD_PATH/packed
UNPACKEDDIR=$BUILD_PATH/unpacked


DIR=$WEB_PATH/pre-public_html/ryzom_live_data/
rm -rf $DIR
mkdir -p $DIR

mkdir -p $DIR/data

clr_green "Unpacking previous data in pre-public..."
cd $DIR/data/
7z x ../../../public_html/ryzom_live_data.7z

clr_green "Copying Ryzom data in web directories..."

cp -r $RYZOMDATA_PATH/client/cfg $DIR
cp -p $PACKEDDIR/*.bnp $DIR/data/
cp -p $PACKEDDIR/*.ref $DIR/data/
rm -f $DIR/data/packedsheets.bnp
rm -f $DIR/data/exedll*.bnp
mkdir -p $DIR/data/fonts
cp -p $UNPACKEDDIR/fonts/* $DIR/data/fonts/
cp -p $UNPACKEDDIR/packedsheets/* $DIR/data/
rm -f $WEB_PATH/pre-public_html/ryzom_live_data.7z

clr_green "Compressing..."
7z a -mx9 -ms=off -r $WEB_PATH/pre-public_html/ryzom_live_data.7z $DIR/*



