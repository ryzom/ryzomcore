#!/bin/bash


rm nohup.out
for dag in Deepl_Shard_Command Deepl_Translator_EN Deepl_Translator_FR Deepl_Translator_DE Deepl_Translator_ES Deepl_Translator_RU Deepl_Zulip_Dispatcher Deepl_Zulip_Fetcher Deepl_Ios_Dispatcher Deepl_Ios_Fetcher
do
	dagu -c /etc/dagu.yaml stop /home/data/dagu/dags/${dag}.yaml
done

for script in shard_commands.py ios_fetcher.py ios_dispatcher.py zulip_fetcher.py zulip_dispatcher.py translator.py
do
	ps aux | grep -v grep | grep "python3 $script"
	pid=$(ps aux | grep -v grep | grep "python3 $script" | awk '{print $2}')
	kill $pid
	echo "Killed!"
done

for dag in Deepl_Shard_Command Deepl_Translator_EN Deepl_Translator_FR Deepl_Translator_DE Deepl_Translator_ES Deepl_Translator_RU Deepl_Zulip_Dispatcher Deepl_Zulip_Fetcher Deepl_Ios_Dispatcher Deepl_Ios_Fetcher
do
	nohup dagu -c /etc/dagu.yaml start /home/data/dagu/dags/${dag}.yaml &> /dev/null &
done
