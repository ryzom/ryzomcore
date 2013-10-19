<?php

/**
* This small piece of php code calls the cron() function of the Mail_Handler. 
* @author Daan Janssens, mentored by Matthew Lagoe
*/

require( '../libinclude.php' );
require( '../../www/config.php' );

$mail_handler = new Mail_Handler();
$mail_handler->cron();