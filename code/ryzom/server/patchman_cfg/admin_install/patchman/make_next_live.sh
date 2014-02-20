#! /bin/sh -

# note: this script should be run from a domain directory such as /srv/core/std01 or /srv/core/mini01
DOMAIN=$(pwd |sed 's/\/srv\/core\///')
if [ "patchman" = "$DOMAIN" ]; then DOMAIN= ; fi
if [ "bin" = "$DOMAIN" ]; then DOMAIN= ; fi
if [ "$DOMAIN" != $(echo $DOMAIN|sed 's/\///g') ]; then DOMAIN= ; fi
if [ _"${DOMAIN}"_ = __ ]
then
    echo This is not a valid directory for running this script
    exit
fi

# tell the aes to shut everybody down
printf "0" > ./global.launch_ctrl

# before entering the 'Waiting for Services' loop, get rid of the ras/ras.state file because the ras doesn't stop properly otherwise
if [ -f ras/ras.state ]
then
	rm ras/ras.state
fi

# while there are still services running, wait
while [ $(grep -i RUNNING . */*.state|wc -l) != 0 ]
do
  echo $DOMAIN: Waiting for $(grep -i RUNNING . */*.state|wc -l) Services to stop
  sleep 2
done

# stop the screen for the shard (if there is one)
screen -drR -S $DOMAIN -X quit> /dev/null
sleep 1

# rename any old core files
for COREFILE in */core*
do
	mv $COREFILE $(echo $COREFILE|sed "s%/.*%%")/v$(cat live/version)_$(echo $COREFILE|sed "s%.*/%%")
done

# rename any old log files
for LOGFILE in */log*.log
do
	mv $LOGFILE $(echo $LOGFILE|sed "s%/.*%%")/v$(cat live/version)_$(echo $LOGFILE|sed "s%.*/%%")
done

# swap the live and next directories
rm -r old_live/* 2> /dev/null
echo next=$(cat next/version) live=$(cat live/version)
mv live old_live
echo next=$(cat next/version) old_live=$(cat old_live/version)
mv next live
echo old_live=$(cat old_live/version) live=$(cat live/version)
mv old_live next
echo next=$(cat next/version) live=$(cat live/version)

# restore any old log files in case of return to previous version
for LOGFILE in */v$(cat live/version)_log*.log
do
	mv $LOGFILE $(echo $LOGFILE|sed "s%/.*%%")/$(echo $LOGFILE|sed "s%.*/.*_%%")
done

# make the ryzom services executable
chmod 775 live/service_*/*_service 2> /dev/null

# special case to deal with www files that need a local cfg file to be properly setup
if [ -e ./live/data_www/config.php ]
        then
        echo \<?php >./live/data_www/config.php
        echo >>./live/data_www/config.php
        echo \$USERS_DIR = \'$(pwd)/www\'\; >>./live/data_www/config.php
        echo \$TEMPLATE_DIR = \'./template\'\; >>./live/data_www/config.php
        echo >>./live/data_www/config.php
        echo \?\> >>./live/data_www/config.php
        mkdir -p $(pwd)/save_shard/www
fi

# remove any launch ctrl files that are floating about
rm -v */*.*launch_ctrl *.*launch_ctrl 2> /dev/null

# initialise the state files for the new services to "xxxxx" and remove directories that are no longer of interest
for D in $(ls */log.log | sed "s%/.*%%" | sort -u)
do
  if [ $(grep \"$D\" admin_executor_service.cfg | wc -l) == 1 ]
  then
	  printf "xxxxx" > $D/$D.state
  else
	  mkdir -p old
	  mv $D old/
  fi
done

# tell the aes to launch everybody...
printf "1" > ./global.launch_ctrl

# create a script for accessing the screen for this shard
SCRIPT_FILE=/srv/core/bin/${DOMAIN}
echo "#!/bin/sh" > $SCRIPT_FILE
echo "cd "$(pwd) >> $SCRIPT_FILE
echo '/bin/sh /srv/core/bin/ryzom_domain_screen_wrapper.sh $*' >> $SCRIPT_FILE
chmod +x $SCRIPT_FILE

# launch the screen again now that were all done (aes will launch everybody when he comes online)
cp /srv/core/$DOMAIN/${DOMAIN}.screen.rc /srv/core/${DOMAIN}.screen.rc
#screen -S $DOMAIN -d -m -c /srv/core/${DOMAIN}.screen.rc
$SCRIPT_FILE batchstart

