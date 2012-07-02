<?php
	if(!defined('APP_NAME')) {
		die(-1);
	}

	$_CONF = array();

	$_CONF['app_achievements_path'] = "../app_achievements/";
	$_CONF['image_url'] = "http://www.3025-game.de/special/app_achievements/";
	$_CONF['enable_webig'] = true;
	$_CONF['enable_offgame'] = true;
	$_CONF['enable_CSR'] = true;
	$_CONF['enable_ADM'] = true;

	$_CONF['char_mysql_server'] = "localhost";
	$_CONF['char_mysql_user'] = "root";
	$_CONF['char_mysql_pass'] = "";
	$_CONF['char_mysql_database'] = "app_achievements";
?>