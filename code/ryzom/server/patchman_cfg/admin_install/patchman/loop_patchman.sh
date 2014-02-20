#!/bin/sh

while true
do
  cd /srv/core/
  if [ -e /srv/core/admin_install.tgz ]
	  then
	  tar xvzf admin_install.tgz
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
