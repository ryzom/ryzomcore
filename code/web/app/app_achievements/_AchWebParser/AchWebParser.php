<?php
	error_reporting(E_ALL ^ E_NOTICE);
	ini_set("display_errors","1");

	require_once("class/mySQL_class.php");
	require_once("conf.php");
	require_once("inlcude/functions_inc.php");

	$logfile = false;
	if($CONF['logging'] == true) {
		$logfile = fopen($CONF['logfile'].'.'.date("Ymd",time()).'.txt','a');
	}

	//create database connection
	$DBc = new mySQL($CONF['mysql_error']);
	$DBc->connect($CONF['mysql_server'],$CONF['mysql_user'],$CONF['mysql_pass'],$CONF['mysql_database']);

	#MISSING: mode -> single, cron, debug

	require_once("class/DataSourceHandler_class.php");
	require_once("class/DataSource_abstract.php");

	//create datasource handler
	$_DATA = new DataSourceHandler();
	foreach($CONF['data_source'] as $elem) { //populate
		require_once("source/".$elem."/".$elem."_class.php");
		eval('$tmp = new '.$elem.'();');
		$_DATA->registerDataSource($tmp);
	}

	#MISSING: fetch candidates

	foreach() {
		#MISSING: fetch objectives to evaluate
		foreach() {
			#MISSING: fetch atoms
			foreach() {
				#MISSING: evaluate atoms
			}
			#MISSING: evaluate objective
		}
		#MISSING: evaluate perk
	}

	#WORKPAD:####

	Trigger:
		by value
		(by event)

	Sources:
		XML
		valuecache
		(Achievement Service)
			(Mirror Service)

	
	VALUE dappers = c_money
	if(dappers >= 5000) {
		GRANT
	}
	
	VALUE tmp = c_fame[scorchers]
	if(tmp == 0) {
		DENY:3600
	}

	EVENT:player_death
	ON player_death {
		UNLOCK
	}

	#############



	#MISSING: self call on cron mode
	if() {
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