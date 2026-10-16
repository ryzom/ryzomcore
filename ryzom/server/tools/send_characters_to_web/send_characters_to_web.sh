#!/bin/bash

TEMP_PATH=/run/user/1000/chweb
mkdir -p $TEMP_PATH

F=$SHARD_PATH/save/new_save.txt
TP=$TEMP_PATH/toprocess.txt

while inotifywait -e close_write $F; do
	echo "File changed..."
	mv $F $TP
	touch $F

	for SRC in `sort -u $TP`
	do
		SRC=$SHARD_PATH/run/$SRC
		FN=`basename $SRC .bin`
		echo "	Xmlize $FN"
		if [[ ${SRC} != *"offline_commands"* ]]
		then
			cp $SRC $TEMP_PATH/
			pdr2xml $TEMP_PATH/$FN.bin > /dev/null
			if [ ! -e $TEMP_PATH/$FN.xml ]
			then
				# TODO sent mail or rocket message
				echo "BAD !!!"
				exit 1
			fi
		fi
	done
	#rm $TP
	rm $TEMP_PATH/*.bin
	echo "	Sent to app server"	
	scp -p -C -r $TEMP_PATH/* app@app.ryzom.com:/home/app/www/api/server/scripts/achievement_script/new_xml/ 

	rm -f $TEMP_PATH/*
done

