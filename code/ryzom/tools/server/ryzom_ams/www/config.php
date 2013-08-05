<?php

// This file contains all variables needed by other php scripts
// ----------------------------------------------------------------------------------------
// Variables for database access
// ----------------------------------------------------------------------------------------
// where we can find the mysql database
//-----------------------------------------------------------------------------------------

$cfg['db']['web']['host']    = 'localhost';
$cfg['db']['web']['port']    = '3306';
$cfg['db']['web']['name']    = 'ryzom_ams';
$cfg['db']['web']['user']    = 'shard';
$cfg['db']['web']['pass']    = '';

$cfg['db']['lib']['host']    = 'localhost';
$cfg['db']['lib']['port']    = '3306';
$cfg['db']['lib']['name']    = 'ryzom_ams_lib';
$cfg['db']['lib']['user']    = 'shard';
$cfg['db']['lib']['pass']    = '';

$cfg['db']['shard']['host']    = 'localhosti';
$cfg['db']['shard']['port']    = '3306';
$cfg['db']['shard']['name']    = 'nel';
$cfg['db']['shard']['user']    = 'shard';
$cfg['db']['shard']['pass']    = '';

$cfg['db']['ring']['host']    = 'localhost';
$cfg['db']['ring']['port']    = '3306';
$cfg['db']['ring']['name']    = 'ring_open';
$cfg['db']['ring']['user']    = 'shard';
$cfg['db']['ring']['pass']    = '';

//-----------------------------------------------------------------------------------------
// If true= the server will add automatically unknown user in the database
// (in nel.user= nel.permission= ring.ring_user and ring.characters
$ALLOW_UNKNOWN = true ;
// if true= the login service automaticaly create a ring user and a editor character if needed
$CREATE_RING = true ;

 // site paths definitions
$AMS_LIB = dirname( dirname( __FILE__ ) ) . '/ams_lib';
$AMS_TRANS = $AMS_LIB . '/translations';
$AMS_CACHEDIR = $AMS_LIB . '/cache';

$DEFAULT_LANGUAGE = 'en';

$SITEBASE = dirname( __FILE__ ) . '/html/' ;

$TICKET_LOGGING = true;
$TIME_FORMAT = "m-d-Y H:i:s";
$INGAME_LAYOUT = "basic";