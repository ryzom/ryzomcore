<?php
	require_once("../../webig/config.php");

	$CONF = array();

	$CONF['logging'] = true;
	$CONF['logfile'] = "log/AchWebParser.log";

	$CONF['mysql_error'] = "PRINT";
	$CONF['mysql_server'] = RYAPI_WEBDB_HOST;
	$CONF['mysql_user'] = RYAPI_WEBDB_LOGIN;
	$CONF['mysql_pass']	= RYAPI_WEBDB_PASS;
	$CONF['mysql_database'] = "app_achievements_test";

	$CONF['char_mysql_server'] = "localhost";
	$CONF['char_mysql_user'] = "root";
	$CONF['char_mysql_pass'] = "";
	$CONF['char_mysql_database'] = "app_achievements";

	$CONF['data_source'] = array("PDRtoXMLdriver");

	$CONF['facebook'] = false;
	$CONF['fb_id'] = "447985781893176";
	$CONF['fb_secret'] = "f953772f1f7d871db022a6023e7a3f42";
?>