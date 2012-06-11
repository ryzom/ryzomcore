<?php
	error_reporting(E_ALL ^ E_NOTICE);
	ini_set("display_errors","1");

	if(file_exists("monitor.stop")) {
		exit(0);
	}

	require_once("class/mySQL_class.php");
	require_once("conf.php");

	//create database connection
	$DBc = new mySQL($CONF['mysql_error']);
	$DBc->connect($CONF['mysql_server'],$CONF['mysql_user'],$CONF['mysql_pass'],$CONF['mysql_database']);

	//check status
	$res = $DBc->sendSQL("SELECT * FROM ach_monitor_status ORDER by ams_start DESC LIMIT 0,1","ARRAY");

	if(($res[0]['ams_start'] < (time()-$CONF['timeout']) && $res[0]['ams_end'] == 0) || ($res[0]['ams_end'] > 0 && $res[0]['ams_end'] < (time()-$CONF['timeout']))) {
		$fp = fsockopen($CONF['self_host'], 80, $errno, $errstr, 30);
		if(!$fp) {
			logf("ERROR: self call; socket: ".$errstr." (."$errno.")");
		}
		else {
			$out = "GET ".$CONF['self_path']." HTTP/1.1\r\n";
			$out .= "Host: ".$CONF['self_host']."\r\n";
			$out .= "Connection: Close\r\n\r\n";

			fwrite($fp, $out);
			fclose($fp);
		}
	}

	exit(0);
?>