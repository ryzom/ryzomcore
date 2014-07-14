<?php

// This file contains all variables needed by other php scripts

$LogRelativePath = 'logs/';

// ----------------------------------------------------------------------------------------
// Variables for nel database access
// ----------------------------------------------------------------------------------------

// where we can find the mysql database
$DBHost         = "localhost";
$DBUserName     = "shard";
$DBPassword     = "";
$DBName         = "nel";

$RingDBUserName = "shard";
$RingDBName = "ring_open";
$RingDBPassword = "";

// If true, the server will add automatically unknown user in the database
// (in nel.user, nel.permission, ring.ring_user and ring.characters
$AcceptUnknownUser = false;
// if true, the login service automaticaly create a ring user and a editor character if needed
$AutoCreateRingInfo = true;

?>
