#!/bin/sh

SUBID=$1
FILE=$2
cd $SHARD_PATH/run


mkdir -p sc2w

cp $SHARD_PATH/$SUBID/$FILE sc2w/
pdr_util -x -s/$SHARD_PATH/common/data_leveldesign/leveldesign/Game_elem/sheet_id.bin sc2w/$FILE
rm sc2w/$FILE

scp -p -C -r sc2w/* app@app.ryzom.com:/home/api/public_html/server/scripts/achievement_script/new_xml/
rm sc2w/*

