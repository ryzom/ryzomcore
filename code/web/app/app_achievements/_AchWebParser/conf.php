<?php
	$CONF = array();

	$CONF['logging'] = true;
	$CONF['logfile'] = "log/AchWebParser.log";

	$CONF['mysql_error'] = "PRINT";
	$CONF['mysql_server'] = "localhost";
	$CONF['mysql_user'] = "root";
	$CONF['mysql_pass']	= "";
	$CONF['mysql_database'] = "app_achievements";

	$CONF['data_source'] = array("PDRtoXMLdriver");

	/*$CONF['synch_chars'] = true;
	$CONF['char_mysql_server'] = "localhost";
	$CONF['char_mysql_user'] = "root";
	$CONF['char_mysql_pass'] = "";
	$CONF['char_mysql_database'] = "ring_open";*/

	$CONF['fork'] = true;

	$CONF['self_host'] = "127.0.0.1";
	$CONF['self_path'] = "/path/to/AchWebParser.php";

	$CONF['sleep_time'] = 1500;

	$CONF['enable_selfcall'] = true;

	$CONF['timeout'] = 60*60;
?>