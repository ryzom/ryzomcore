#!/bin/sh

echo Launched: $(date)
while true
do
  # retrieve ATS files from ATS admin tool machine
  rsync -t ep1.std01.ryzomcore.org:ats/graph_datas/* /srv/core/mini01/rrd_graphs/

  # deal with live files - duplicate files that correspond to unique services to aid with graphing of su & co
  cd /srv/core/std01/rrd_graphs/
  for f in $(ls *rrd | awk '/^[^_]*\./'); do cp $f $(cut -d. -f1)_unifier.$(cut -d. -f2-); done
  rsync -t /srv/core/std01/rrd_graphs/* csr:std01_rrd_graphs/

  # deal with test files files - see comment regarding live files above
  cd /srv/core/mini01/rrd_graphs/
  for f in $(ls *rrd | awk '/^[^_]*\./'); do cp $f $(echo $f|cut -d. -f1)_unifier.$(echo $f|cut -d. -f2-); done
  rsync -t /srv/core/mini01/rrd_graphs/* csr:mini01_rrd_graphs/

  # display a groovy message
  echo Finished rsync: $(date)
  sleep 60
done
