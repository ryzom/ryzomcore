#!/bin/bash
cd /home/data/dagu/dags

STATUS=$(cat /home/nevrax/www/login/server_open_status)

if [ "$STATUS" = "ds_dev" -o "$STATUS" = "ds_closed" ]; then
	echo "Exit because shard is closed"
	exit 1
fi
shard=$(hostname | cut -d"." -f1)

nohup dagu start "${shard^}_$1" -c /etc/dagu.yaml &
