#!/bin/bash
cd /home/data/dagu/dags

STATUS=$(cat /home/nevrax/www/login/server_open_status)

if [[ "$STATUS" == "ds_dev" ]] || [[ "$STATUS" == "ds_closed" ]]
then
	echo "Exit because shard is closed"
	exit -1
fi

dagu enqueue Ryzom_$1 -c /etc/dagu.yaml
