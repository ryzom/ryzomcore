#!/bin/bash
for script in shard_commands.py ios_fetcher.py ios_dispatcher.py zulip_fetcher.py zulip_dispatcher.py translator.py
do
	ps aux | grep -v grep | grep "python3 $script"
	pid=$(ps aux | grep -v grep | grep "python3 $script" | awk '{print $2}')
	kill $pid
	echo "Killed!"
done
