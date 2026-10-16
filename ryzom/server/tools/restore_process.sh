#!/bin/sh

echo Launched: $(date)

cd /home/nevrax/tmp/move/
while true
do
	inotifywait -e close_write from_appadmin
	sh from_appadmin > out.log
	echo "" > from_appadmin
done

