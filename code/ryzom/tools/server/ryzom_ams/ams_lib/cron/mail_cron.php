<?php

require( '../libinclude.php' );
require( '../../www/config.php' );

$mail_handler = new Mail_Handler();
$mail_handler->cron();