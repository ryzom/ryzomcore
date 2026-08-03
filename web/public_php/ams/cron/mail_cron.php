<?php

/**
* This small piece of php code calls the cron() function of the Mail_Handler. 
* @author Daan Janssens, mentored by Matthew Lagoe
*/

require( '../../config.php' );
require_once( $AMS_LIB . '/libinclude.php' );

// Meant to be run from cron. Over http it opens mailboxes and talks to the
// shard, so anyone who can reach the url can make it work on demand; require
// a session there. (index.php already runs the lazy sync on page load.)
if (PHP_SAPI !== 'cli') {
	session_start();
	if (!isset($_SESSION['ticket_user']) || !Ticket_User::isAdmin(unserialize($_SESSION['ticket_user']))) {
		header('HTTP/1.1 403 Forbidden');
		echo 'Access denied';
		return;
	}
};

$mail_handler = new Mail_Handler();
$mail_handler->cron();