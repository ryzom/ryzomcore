#!/bin/sh

# Check if screen is runnning
cd $(dirname "$0")/services

SHARD=$(hostname -s)

if [ $(screen -list | grep \\\.services | wc -l) != 1 ]
then
	echo "Services Screen not running. Starting..."
	screen -dmS services -c ../screen.rc
else
	echo "Good! Services Screen is running"
fi

# Check if each services is running in a screen window
WINDOWS=$(screen -S services -Q windows)
for service in *
do
	CHECK=$(echo "$WINDOWS" | grep "$service")
	if [ -z "$CHECK" ]
	then
		echo "Service '$service' need be started! Do it..."
		screen -S services -p bash -X stuff "cd ~/scripts/_startup/$SHARD && screen -t $service ./$service\n"
	else
		echo "Good! Service '$service' is up"
	fi
done

