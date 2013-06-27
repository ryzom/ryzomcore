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
$cfg['db']['web']['user']    = 'root';
$cfg['db']['web']['pass']    = 'lol123';

$cfg['db']['lib']['host']    = 'localhost';
$cfg['db']['lib']['port']    = '3306';
$cfg['db']['lib']['name']    = 'ryzom_ams_lib';
$cfg['db']['lib']['user']    = 'root';
$cfg['db']['lib']['pass']    = 'lol123';

$cfg['db']['shard']['host']    = 'localhost';
$cfg['db']['shard']['port']    = '3306';
$cfg['db']['shard']['name']    = 'nel';
$cfg['db']['shard']['user']    = 'shard';
$cfg['db']['shard']['pass']    = '';

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
