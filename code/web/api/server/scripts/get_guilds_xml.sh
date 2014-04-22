#!/bin/sh

cd /home/api/public_html/server/scripts

# get guilds

rm /home/api/public_html/data/cache/guilds/*

rsync -az --rsh=ssh nevrax@shard.ryzom.com:/home/nevrax/code/ryzom/server/save_shard/live_atys/guilds/guild_*.bin /home/api/public_html/data/cache/guilds/

./pdr_util -x -s/home/app/web_hg/api/server/scripts/sheet_id.bin /home/api/public_html/data/cache/guilds/guild_*.bin

rm /home/api/public_html/data/cache/guilds/guild_*.bin

/usr/bin/php ./create_guilds_xml.php

rm /home/api/public_html/data/cache/guilds/guild_?????.xml

rm log.log

/root/bin/own.sh api /home/api/public_html/data/cache/guilds/

# get tick

rsync -az --rsh=ssh nevrax@shard.ryzom.com:/home/nevrax/code/ryzom/server/save_shard/live_atys/game_cycle.ticks /home/api/public_html/data/cache/

chown api:api /home/api/public_html/data/cache/game_cycle.ticks

cd -
