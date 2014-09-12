<?php

// This file contains all variables needed by other php scripts

require_once('../config.php');

$LogRelativePath = 'logs/';

// ----------------------------------------------------------------------------------------
// Variables for nel database access
// ----------------------------------------------------------------------------------------

if ($cfg['db']['shard']['host'] !=  $cfg['db']['ring']['host'])
	throw new Exception("Invalid configuration");

// where we can find the mysql database
$DBHost         = $cfg['db']['shard']['host'];
$DBUserName     = $cfg['db']['shard']['user'];
$DBPassword     = $cfg['db']['shard']['pass'];
$DBName         = $cfg['db']['shard']['name'];

$RingDBUserName = $cfg['db']['ring']['user'];
$RingDBPassword = $cfg['db']['ring']['pass'];

// If true, the server will add automatically unknown user in the database
// (in nel.user, nel.permission, ring.ring_user and ring.characters
$AcceptUnknownUser = $ALLOW_UNKNOWN;
// If true, the login service automaticaly create a ring user and a editor character if needed
$AutoCreateRingInfo = $CREATE_RING;

?>
