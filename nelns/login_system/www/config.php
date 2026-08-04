<?php

// This file contains all variables needed by other php scripts

// php 8.1 defaults mysqli to exception reporting; index.php checks its
// results with `or die`, keep the classic mode
if (function_exists('mysqli_report'))
	mysqli_report(MYSQLI_REPORT_OFF);

// ---------------------------------------------------------------------------------------- 
// Variables for index.php
// ---------------------------------------------------------------------------------------- 

	// where we can find the mysql database
	$DBHost		= "localhost";
	$DBPort		= 0;		// 0 means the default mysql port
	$DBUserName	= "nel";
	$DBPassword	= "";
	$DBName		= "nel";

	// If true, the server will add automatically unknown user in the database
	$AcceptUnknownUser = false;

// ---------------------------------------------------------------------------------------- 
// Variables for service_connection.inc
// ---------------------------------------------------------------------------------------- 

	$LSHost = "compilo";
	$LSPort = 49998;

?>
