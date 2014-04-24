#!/bin/sh

CFGFILENAME=patchman_service.${SERVER_TYPE}.cfg
echo cfg file: $CFGFILENAME

AESCFGFILENAME=admin_executor_service_default.${SERVER_TYPE}.cfg
echo aes cfg file: $AESCFGFILENAME

cd /srv/core/patchman
if [ -e $CFGFILENAME ]
	then

	# setup the config file for the patchman
	echo Using configuration file: $CFGFILENAME
	cp $CFGFILENAME patchman_service.cfg

	# setup the config file for the admin executor service
	echo Using aes configuration file: $AESCFGFILENAME
	if [ -e $AESCFGFILENAME ] ; then cp $AESCFGFILENAME admin_executor_service_default.cfg ; fi

	# start the patchman service
	echo Launching patchman...
	./ryzom_patchman_service -C. -L.

	sleep 2
	if [ -e core* ]
	then
		if [ -e dont_keep_cores ]
		then
			rm core*
		fi
	fi

else
	echo ERROR: Failed to locate config file: $CFGFILENAME
	echo trying again in a few seconds...
	sleep 10
fi
cd -
