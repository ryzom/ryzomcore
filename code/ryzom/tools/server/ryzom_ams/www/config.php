<?php

// This file contains all variables needed by other php scripts
// ----------------------------------------------------------------------------------------
// Variables for database access
// ----------------------------------------------------------------------------------------
// where we can find the mysql database
$DBHOST = 'localhost' ;
 $DBNAME = 'nel' ;
 $DBUSERNAME = 'shard' ;
 $DBPASSWORD = '' ;

 $RINGDBNAME = 'ring_open' ;
 $RINGDBUSERNAME = 'shard' ;
 $RINGDBPASSWORD = '' ;

 $NTDBName = 'nel_tool' ;
 $NTUserName = 'shard' ;
 $NTPassword = '' ;

 $SITETITLE = 'Ryzom Core AMS' ;

 $LOGRELATIVEPATH = 'logs/' ;

// If true= the server will add automatically unknown user in the database
// (in nel.user= nel.permission= ring.ring_user and ring.characters
$ALLOW_UNKNOWN = true ;
// if true= the login service automaticaly create a ring user and a editor character if needed
$CREATE_RING = true ;

 // site paths definitions
$AMS_LIB = dirname(dirname( __FILE__ )) . '/ams_lib';
$NELTOOL_SITEBASE = dirname( __FILE__ ) . '/html/' ;
 $NELTOOL_SYSTEMBASE = dirname( dirname( __FILE__ ) ) . '/admin/' ;
 $NELTOOL_LOGBASE = $NELTOOL_SYSTEMBASE . '/logs/' ;
 $NELTOOL_IMGBASE = $NELTOOL_SYSTEMBASE . '/imgs/' ;

 
 $NELTOOL_RRDTOOL = '/usr/bin/rrdtool' ;
 $NELTOOL_RRDSYSBASE = $NELTOOL_SYSTEMBASE . 'graphs_output/' ;
 $NELTOOL_RRDWEBBASE = $NELTOOL_SITEBASE . 'graphs_output/' ;

 // SQL table names
$NELDB_PREFIX = 'neltool_' ;

 // for later use
// the config table will gather some of the settings
// that are currently written in this config.php file
$NELDB_CONFIG_TABLE = $NELDB_PREFIX . 'config';
 $NELDB_USER_TABLE = $NELDB_PREFIX . 'users' ;
 $NELDB_GROUP_TABLE = $NELDB_PREFIX . 'groups' ;

 $NELDB_LOG_TABLE = $NELDB_PREFIX . 'logs' ;
 $NELDB_NOTE_TABLE = $NELDB_PREFIX . 'notes' ;

 $NELDB_STAT_HD_TIME_TABLE = $NELDB_PREFIX . 'stats_hd_times' ;
 $NELDB_STAT_HD_TABLE = $NELDB_PREFIX . 'stats_hd_datas' ;

 $NELDB_ANNOTATION_TABLE = $NELDB_PREFIX . 'annotations' ;
 $NELDB_LOCK_TABLE = $NELDB_PREFIX . 'locks' ;

 $NELDB_APPLICATION_TABLE = $NELDB_PREFIX . 'applications' ;
 $NELDB_GROUP_APPLICATION_TABLE = $NELDB_PREFIX . 'group_applications' ;
 $NELDB_USER_APPLICATION_TABLE = $NELDB_PREFIX . 'user_applications' ;

 $NELDB_DOMAIN_TABLE = $NELDB_PREFIX . 'domains' ;
 $NELDB_USER_DOMAIN_TABLE = $NELDB_PREFIX . 'user_domains' ;
 $NELDB_GROUP_DOMAIN_TABLE = $NELDB_PREFIX . 'group_domains' ;

 $NELDB_SHARD_TABLE = $NELDB_PREFIX . 'shards' ;
 $NELDB_USER_SHARD_TABLE = $NELDB_PREFIX . 'user_shards' ;
 $NELDB_GROUP_SHARD_TABLE = $NELDB_PREFIX . 'group_shards' ;

 $NELDB_RESTART_GROUP_TABLE = $NELDB_PREFIX . 'restart_groups' ;
 $NELDB_RESTART_MESSAGE_TABLE = $NELDB_PREFIX . 'restart_messages' ;
 $NELDB_RESTART_SEQUENCE_TABLE = $NELDB_PREFIX . 'restart_sequences' ;

 $VIEW_DELAY = 0 ;
 $HARDWARE_REFRESH = 600 ;
 $LOCK_TIMEOUT = 1800 ;
 $BG_IMG = 'imgs/bg_live.png' ;
 $GAME_NAME = 'Ryzom Core';
$WELCOME_MESSAGE = 'Welcome! Please fill in the following fields to get your new '.$GAME_NAME.' account:';

$TEMPLATE_DIR = "";