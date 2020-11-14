
cd /home/nevrax/code/ryzom/server

shard stop

clean_log.sh

rm src/*/*.packed_sheets

make

shard batchstart
