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
