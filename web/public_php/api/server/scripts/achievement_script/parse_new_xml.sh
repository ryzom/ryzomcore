#!/bin/sh

cd /home/api/public_html/server/scripts/achievement_script

SRC=/home/api/public_html/server/scripts/achievement_script/new_xml
DST=/home/api/public_html/server/scripts/achievement_script/parse_xml

while true; do

	FN=`inotifywait -r -e close_write --format '%w%f' $SRC`

    for f in $SRC/*; do
		NAME="$DST/"`basename $f`
		mv -f $f $DST
		echo "Processing $NAME"
		php AchWebParser.php $NAME
		rm -f $NAME
	done

done

#cd -
