<?php
/**
* This file contains all variables needed by other php scripts.
* @author Daan Janssens, mentored by Matthew Lagoe
*/

// Variables for database access to the CMS/WWW database
$cfg['db']['web']['host']    = variable_get('ryzommanage_webserverurl', 'localhost');
$cfg['db']['web']['port']    = variable_get('ryzommanage_webmysqlport', '3306');
$cfg['db']['web']['name']    = variable_get('ryzommanage_webdbname', 'drupal');
$cfg['db']['web']['user']    = variable_get('ryzommanage_webusername', 'shard');
$cfg['db']['web']['pass']    = variable_get('ryzommanage_webpassword', '');

// Variables for database access to the lib database
$cfg['db']['lib']['host']    = variable_get('ryzommanage_libserverurl', 'localhost');
$cfg['db']['lib']['port']    = variable_get('ryzommanage_libmysqlport', '3306');
$cfg['db']['lib']['name']    = variable_get('ryzommanage_libdbname', 'ryzom_ams_lib');
$cfg['db']['lib']['user']    = variable_get('ryzommanage_libusername', 'shard');
$cfg['db']['lib']['pass']    = variable_get('ryzommanage_libpassword', '');

// Variables for database access to the shard database
$cfg['db']['shard']['host']    = variable_get('ryzommanage_shardserverurl', 'localhost');
$cfg['db']['shard']['port']    = variable_get('ryzommanage_shardmysqlport', '3306');
$cfg['db']['shard']['name']    = variable_get('ryzommanage_sharddbname', 'nel');
$cfg['db']['shard']['user']    = variable_get('ryzommanage_shardusername', 'shard');
$cfg['db']['shard']['pass']    = variable_get('ryzommanage_shardpassword', '');

// Variables for database access to the open_ring database
$cfg['db']['ring']['host']    = variable_get('ryzommanage_ringserverurl', 'localhost');
$cfg['db']['ring']['port']    = variable_get('ryzommanage_ringmysqlport', '3306');
$cfg['db']['ring']['name']    = variable_get('ryzommanage_ringdbname', 'ring_open');
$cfg['db']['ring']['user']    = variable_get('ryzommanage_ringusername', 'shard');
$cfg['db']['ring']['pass']    = variable_get('ryzommanage_ringpassword', '');

// To connect to an IMAP server running on port 143 on the local machine,
// do the following: $mbox = imap_open("{localhost:143}INBOX", "user_id", "password");       
// POP3 server on port 110: $mbox = imap_open ("{localhost:110/pop3}INBOX", "user_id", "password");      
// SSL IMAP or POP3 server, add /ssl after the protocol:  $mbox = imap_open ("{localhost:993/imap/ssl}INBOX", "user_id", "password");   
// To connect to an SSL IMAP or POP3 server with a self-signed certificate,
// add /ssl/novalidate-cert after the protocol specification:
// $mbox = imap_open ("{localhost:995/pop3/ssl/novalidate-cert}", "user_id", "password");       
// NNTP server on port 119 use: $nntp = imap_open ("{localhost:119/nntp}comp.test", "", "");
// To connect to a remote server replace "localhost" with the name or the IP address of the server you want to connect to.
//$cfg['mail']['server'] = '{localhost:110/pop3/novalidate-cert}INBOX';
$cfg['mail']['default_mailserver']= '{imap.gmail.com:993/imap/ssl}INBOX';
$cfg['mail']['default_groupemail'] = 'amsryzom@gmail.com';
$cfg['mail']['default_groupname'] = 'Ryzomcore Support';
$cfg['mail']['default_username']    = 'amsryzom@gmail.com';
$cfg['mail']['default_password']    = 'lol123bol';
$cfg['mail']['host'] = "ryzomcore.com";

//Defines mailing related stuff
$SUPPORT_GROUP_IMAP_CRYPTKEY = "azerty";
$TICKET_MAILING_SUPPORT = false;

//You have to create this dir at first!
$MAIL_DIR = "/tmp/mail";
$MAIL_LOG_PATH = "/tmp/mail/cron_mail.log";

//crypt is being used by encrypting & decrypting of the IMAP password of the supportgroups
$cfg['crypt']['key']    = 'Sup3rS3cr3tStuff';
$cfg['crypt']['enc_method']    = 'AES-256-CBC';
$cfg['crypt']['hash_method'] = "SHA512";

//terms of service url location
$TOS_URL = variable_get('ryzommanage_TOS', 'www.mytosurlhere.com');
//-----------------------------------------------------------------------------------------
// If true= the server will add automatically unknown user in the database
// (in nel.user= nel.permission= ring.ring_user and ring.characters
$ALLOW_UNKNOWN = true ;
// if true= the login service automaticaly create a ring user and a editor character if needed
$CREATE_RING = true ;

 // site paths definitions
$AMS_LIB =  dirname( __FILE__ )  . '/ams_lib';
$AMS_TRANS = $AMS_LIB . '/translations';
$AMS_CACHEDIR = $AMS_LIB . '/cache';
$SITEBASE = dirname( __FILE__ );
$BASE_WEBPATH = 'http://localhost:40917/drupal';
$IMAGELOC_WEBPATH = $BASE_WEBPATH. '/sites/all/modules/ryzommanage/ams_lib/img' ;
$WEBPATH = $BASE_WEBPATH .'/ams';
$INGAME_WEBPATH = $BASE_WEBPATH . '/ingame';
$CONFIG_PATH = dirname( __FILE__ );

//defines the default language
$DEFAULT_LANGUAGE = 'en';

//defines if logging actions should happen or not.
$TICKET_LOGGING = true;

//defines the time format display
$TIME_FORMAT = "m-d-Y H:i:s";

//defines which ingame layout template should be used
$INGAME_LAYOUT = "basic";

//forces to load the ingame templates if set to true
$FORCE_INGAME = false;



