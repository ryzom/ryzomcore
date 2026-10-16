#!/bin/sh

echo Launched: $(date)
while true
do
  cd $SHARD_PATH/save/rrd_graphs

  rsync -t * app@app.ryzom.com:/home/admin.atrium/rrd_graphs/

  # display a groovy message
  echo Finished rsync: $(date)
  sleep 60
done
