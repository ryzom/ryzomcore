<?php

/**
* This small piece of php code calls the cron() function of the Mail_Handler. 
* @author Daan Janssens, mentored by Matthew Lagoe
*/

require( '../../config.php' );
require_once( $AMS_LIB . '/libinclude.php' );;

$mail_handler = new Mail_Handler();
$mail_handler->cron();