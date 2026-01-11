<?php
    // necessary to include the server api
    $_SERVER['HTTP_HOST'] = 'app.ryzom.com';

	require_once("../../config.php");

	$CONF = array();

	$CONF['logging'] = true;
	$CONF['logfile'] = "/log/";

	$CONF['mysql_error'] = "LOG";
	$CONF['mysql_server'] = RYAPI_WEBDB_HOST;
	$CONF['mysql_user'] = RYAPI_WEBDB_LOGIN;
	$CONF['mysql_pass']	= RYAPI_WEBDB_PASS;
	$CONF['mysql_database'] = "app_achievements";

	$CONF['webig_mysql_database'] = "webig";

	$CONF['char_mysql_server'] = RYAPI_NELDB_HOST;
	$CONF['char_mysql_user'] = RYAPI_NELDB_LOGIN;
	$CONF['char_mysql_pass'] = RYAPI_NELDB_PASS;
	$CONF['char_mysql_database'] = RYAPI_NELDB_RING;

	$CONF['export_xml_path'] = "../../../data/cache/players/";
	#$CONF['export_xml_path'] = "cache/players/";

	$CONF['data_source'] = array("PDRtoXMLdriver");

	$CONF['facebook'] = false;
	$CONF['fb_id'] = "447985781893176";
	$CONF['fb_secret'] = "f953772f1f7d871db022a6023e7a3f42";
?>