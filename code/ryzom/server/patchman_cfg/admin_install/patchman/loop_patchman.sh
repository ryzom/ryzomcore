#!/bin/sh

while true
do
  cd /srv/core/
  if [ -e /srv/core/admin_install.tgz ]
	  then
	  tar xvzf admin_install.tgz
	  chmod 775 bin/admin 2> /dev/null
	  chmod 775 bin/ps_services 2> /dev/null
	  chmod 775 bin/run_forever 2> /dev/null
	  chmod 775 bin/shard 2> /dev/null
	  chmod 775 bin/domain_* 2> /dev/null
	  chmod 775 bin/startup 2> /dev/null
	  chmod 775 bin/*.sh 2> /dev/null
	  chmod 775 patchman/*_service 2> /dev/null
	  chmod 775 patchman/*.sh 2> /dev/null
  fi

  cd /srv/core/patchman/
  if [ $(grep $(hostname) patchman_list |wc -l) -gt 0 ]
	  then
	  export SERVER_TYPE=$(grep $(hostname) patchman_list | awk '{ print $1 }')
  elif [ $(grep $(hostname -s) patchman_list |wc -l) -gt 0 ]
	  then
	  export SERVER_TYPE=$(grep $(hostname -s) patchman_list | awk '{ print $1 }')
  elif [ $(grep $(hostname -d) patchman_list |wc -l) -gt 0 ]
	  then
	  export SERVER_TYPE=$(grep $(hostname -d) patchman_list | awk '{ print $1 }')
  else
	  export SERVER_TYPE=default
	  echo "ERROR: Neither \'hostname\' \($(hostname)\) nor \'hostname -s\' \($(hostname -s)\) nor \'hostname -d\' \($(hostname -d)\) found in $(pwd)/patchman_list"
  fi
  CFGFILENAME=patchman_service.${SERVER_TYPE}.cfg
  
  if [ ! -e $CFGFILENAME ]
	  then
	  echo ERROR: Failed to locate the following file: $CFGFILENAME
	  echo using default files
	  export SERVER_TYPE=default
	  CFGFILENAME=patchman_service.${SERVER_TYPE}.cfg
  	  
	  if [ ! -e $CFGFILENAME ]
		  then
		  echo ERROR: Failed to locate the following DEFAULT file: $CFGFILENAME
		  echo "press enter"
		  read toto
		  exit
	  fi
  fi
  
  echo ssh keys file: $KEYSFILENAME
  echo cfg file: $CFGFILENAME
    
  /bin/sh loop_patchman_once.sh
done
