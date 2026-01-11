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

	$_CONF['char_mysql_server'] = RYAPI_NELDB_HOST;
	$_CONF['char_mysql_user'] = RYAPI_NELDB_LOGIN;
	$_CONF['char_mysql_pass'] = RYAPI_NELDB_PASS;
	$_CONF['char_mysql_database'] = RYAPI_NELDB_RING;

	$_CONF['langs'] = array('en','de','fr','es','ru');
?>