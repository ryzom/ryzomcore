#!/bin/bash
echo "0" > /tmp/killras.count
while true
do
	RAS_PID=$(ps aux | grep "/sbin/ryzom_admin_service --" | grep -v "bash" | grep -v grep | awk '{print $2}')
	RAS_CPU=$(top -b -n 1 -p $RAS_PID | tail -1 | awk '{print $9}' | cut -d"," -f1)
	echo -n "$RAS_CPU "
	
	count=$(cat /tmp/killras.count)
	if (( count > 5 ))
	then
		date
		echo "$RAS_PID : KILLED!"
		kill $RAS_PID
		echo "0" > /tmp/killras.count
	else
		date
		echo "$RAS_PID $RAS_CPU : HOT"
		let count++
		echo "$count" > /tmp/killras.count
	fi
	sleep 1
done
