<?php
	error_reporting(E_ALL ^ E_NOTICE);
	ini_set("display_errors","1");

	if(file_exists("parser.stop")) {
		exit(0);
	}

	require_once("class/mySQL_class.php");
	require_once("conf.php");
	require_once("inlcude/functions_inc.php");

	$logfile = false;
	if($CONF['logging'] == true) {
		require_once("class/Logfile_class.php");
		#$logfile = fopen($CONF['logfile'].'.'.date("Ymd",time()).'.txt','a');
		$logfile = new Logfile($CONF['logfile']);
	}

	//set mode: cron || single with given cid
	#MISSING: conf to allow external calls; whitelist ips
	$MODE = "CRON";
	if($_REQUEST["cid"] > 0 || $_REQUEST["invoke"] == "TRUE") {
		if($_REQUEST["cid"] > 0 && $_REQUEST["invoke"] == "TRUE") {
			$MODE = "SINGLE";
			$CID = $DBc->mre($_REQUEST["cid"]);
		}
		else {
			$e = "Failed to start SINGLE mode; cid=".$_REQUEST["cid"];
			logf($e);
			die($e);
		}
	}

	//create database connection
	$DBc = new mySQL($CONF['mysql_error']);
	$DBc->connect($CONF['mysql_server'],$CONF['mysql_user'],$CONF['mysql_pass'],$CONF['mysql_database']);

	if($MODE == "CRON") {
		$RID = $DBc->sendSQL("INSERT INTO ach_monitor_state (ams_start,ams_end) VALUES ('".time()."','0')","INSERT"); // insert run into monitoring table
	}

	require_once("class/DataSourceHandler_class.php");
	require_once("class/DataSource_abstract.php");

	require_once("class/Atom_class.php");

	//create datasource handler
	$_DATA = new DataSourceHandler();
	foreach($CONF['data_source'] as $elem) { //populate
		require_once("source/".$elem."/".$elem."_class.php");
		eval('$tmp = new '.$elem.'();');
		$_DATA->registerDataSource($tmp);
	}

	//synch chars from ring_open character table
	if($CONF['synch_chars'] == true) {
		$DBc_char = new mySQL($CONF['mysql_error']);
		$DBc_char->connect($CONF['char_mysql_server'],$CONF['char_mysql_user'],$CONF['char_mysql_pass'],$CONF['char_mysql_database']);

		$DBc->sendSQL("UPDATE ach_monitor_character SET amc_confirmed='0'","NONE");

		$res = $DBc_char->sendSQL("SELECT char_id,last_played_date FROM characters","ARRAY");
		$sz = sizeof($res);
		for($i=0;$i<$sz;$i++) {
			$DBc->sendSQL("INSERT INTO ach_monitor_character (amc_character,amc_last_import,amc_last_login,amc_confirmed) VALUES ('".$res[$i]['char_id']."','0','".dateTime_to_timestamp($res[$i]['last_played_date'])."','1') ON DUPLICATE KEY UPDATE amc_confirmed='1', amc_last_login='".dateTime_to_timestamp($res[$i]['last_played_date'])."'","NONE");
		}

		$DBc->sendSQL("DELETE FROM ach_monitor_character WHERE amc_confirmed='0'","NONE"); //remove deleted characters
		//remove data for deleted chars
		$DBc->sendSQL("DELETE FROM ach_player_atom WHERE NOT EXISTS (SELECT * FROM ach_monitor_character WHERE amc_character='apa_player')","NONE");
		$DBc->sendSQL("DELETE FROM ach_player_objective WHERE NOT EXISTS (SELECT * FROM ach_monitor_character WHERE amc_character='apo_player')","NONE");
		$DBc->sendSQL("DELETE FROM ach_player_perk WHERE NOT EXISTS (SELECT * FROM ach_monitor_character WHERE amc_character='app_player')","NONE");
		$DBc->sendSQL("DELETE FROM ach_player_valuecache WHERE NOT EXISTS (SELECT * FROM ach_monitor_character WHERE amc_character='apv_player')","NONE");
	}

	// fetch candidates
	if($MODE == "SINGLE") {
		$chars = array();
		$chars[] = array('amc_character',$CID);
	}
	else {
		#$chars = array();

		$DBc->sendSQL("UPDATE ach_monitor_character SET amc_working='0' WHERE amc_last_import<'".(time()-60*60)."'"); // unlock if something went wrong

		$DBc->sendSQL("UPDATE ach_monitor_character SET amc_working='".$RID."' WHERE amc_last_login>amc_last_import AND amc_working='0'","NONE");
		
		$chars = $DBc->sendSQL("SELECT amc_character FROM ach_monitor_character WHERE amc_working='".$RID."'","ARRAY");
	}


	//fork if enabled in conf
	if($CONF['fork'] == true && $MODE == "CRON") {
		require_once("class/ParallelCURL_class.php");

		$max_requests = 0;
		$curl_options = array(
			CURLOPT_SSL_VERIFYPEER => FALSE,
			CURLOPT_SSL_VERIFYHOST => FALSE,
			CURLOPT_USERAGENT, 'Ryzom - Achievement Tracker',
		);

		$_CURL = new ParallelCurl($max_requests, $curl_options);
		
		foreach($chars as $elem) {
			$_CURL->startRequest("http://".$CONF['self_host']."/".$CONF['self_path']."?invoke=TRUE&cid=".$elem['amc_character'], 'received_char',null);
		}
	}
	else {
		$atom_list = array();

		foreach($chars as $elem) {
			$_DATA->freeData($elem['amc_character']);

			#STEP 1: evaluate atoms

			//get unfinished perks which have no parent or complete parent
			#$res = $DBc->sendSQL("SELECT ap_id FROM ach_perk WHERE (ap_parent IS NULL OR EXISTS (SELECT * FROM ach_player_perk WHERE app_player='".$elem['amc_character']."' AND app_perk=ap_parent)) AND (NOT EXISTS (SELECT * FROM ach_player_perk WHERE app_player='".$elem['amc_character']."' AND app_perk=ap_id))","ARRAY");

			//get all unfinished perks since perks my not directly inherit objectives...
			$res = $DBc->sendSQL("SELECT ap_id FROM ach_perk WHERE NOT EXISTS (SELECT * FROM ach_player_perk WHERE app_player='".$elem['amc_character']."' AND app_perk=ap_id)","ARRAY");
			foreach($res as $perk) {
				//get unfinished atoms belonging to unfinished objectives
				$res = $DBc->sendSQL("SELECT ach_atom.* FROM ach_atom,ach_objective WHERE ao_perk='".$perk['ap_id']."' AND ao_id=atom_objective AND NOT EXISTS (SELECT * FROM ach_player_objective WHERE apo_player='".$elem['amc_character']."' AND apo_objective=ao_id)","ARRAY");
				foreach($res2 as $atom) {
					if(!isset($atom_list[$atom['atom_id']])) { // only load if not already cached
						$atom_list[$atom['atom_id']] = new Atom($atom);
					}

					$atom_list[$atom['atom_id']]->evalRuleset($elem['amc_character']);
				}
			}

			$_DATA->freeData($elem['amc_character']);

			$DBc->sendSQL("UPDATE ach_monitor_character SET amc_last_import='".time()."', amc_working='0' WHERE amc_character='".$elem['amc_character']."' AND amc_working='".$RID."'","NONE");

			#STEP 2: detect obj/perk progression
			//obj
			$res = $DBc->sendSQL("SELECT ao_id FROM ach_objective WHERE ao_condition='all' AND NOT EXISTS (SELECT * FROM ach_atom WHERE atom_objective=ao_id AND NOT EXISTS (SELECT * FROM ach_player_atom WHERE apa_atom=atom_id AND apa_state!='GRANT' AND apa_player='".$elem['amc_character']."'))","ARRAY");
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$DBc->sendSQL("INSERT INTO ach_player_objective (apo_objective,apo_player,apo_date) VALUES ('".$res[$i]['ao_id']."','".$elem['amc_character']."','".time()."')","NONE");
			}

			$res = $DBc->sendSQL("SELECT ao_id FROM ach_objective WHERE ao_condition='value' AND ao_value<=(SELECT count(*) FROM ach_atom WHERE atom_objective=ao_id AND EXISTS (SELECT * FROM ach_player_atom WHERE apa_atom=atom_id AND apa_state='GRANT' AND apa_player='".$elem['amc_character']."'))","ARRAY");
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$DBc->sendSQL("INSERT INTO ach_player_objective (apo_objective,apo_player,apo_date) VALUES ('".$res[$i]['ao_id']."','".$elem['amc_character']."','".time()."')","NONE");
			}

			$res = $DBc->sendSQL("SELECT ao_id FROM ach_objective WHERE ao_condition='any' AND EXISTS (SELECT * FROM ach_atom WHERE atom_objective=ao_id AND EXISTS (SELECT * FROM ach_player_atom WHERE apa_atom=atom_id AND apa_state='GRANT' AND apa_player='".$elem['amc_character']."'))","ARRAY");
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$DBc->sendSQL("INSERT INTO ach_player_objective (apo_objective,apo_player,apo_date) VALUES ('".$res[$i]['ao_id']."','".$elem['amc_character']."','".time()."')","NONE");
			}

			//perk
			$res = $DBc->sendSQL("SELECT ap_id FROM ach_perk WHERE ap_condition='all' AND NOT EXISTS (SELECT * FROM ach_objective WHERE ao_perk=ap_id AND NOT EXISTS (SELECT * FROM ach_player_objective WHERE apo_objective=ao_id AND apo_state!='GRANT' AND apo_player='".$elem['amc_character']."'))","ARRAY");
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$DBc->sendSQL("INSERT INTO ach_player_perk (app_objective,app_player,app_date) VALUES ('".$res[$i]['ap_id']."','".$elem['amc_character']."','".time()."')","NONE");
			}

			$res = $DBc->sendSQL("SELECT ap_id FROM ach_perk WHERE ap_condition='value' AND ap_value<=(SELECT count(*) FROM ach_objective WHERE ao_perk=ap_id AND EXISTS (SELECT * FROM ach_player_objective WHERE apo_objective=ao_id AND apo_state='GRANT' AND apo_player='".$elem['amc_character']."'))","ARRAY");
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$DBc->sendSQL("INSERT INTO ach_player_perk (app_objective,app_player,app_date) VALUES ('".$res[$i]['ap_id']."','".$elem['amc_character']."','".time()."')","NONE");
			}

			$res = $DBc->sendSQL("SELECT ap_id FROM ach_perk WHERE ap_condition='any' AND EXISTS (SELECT * FROM ach_objective WHERE ao_perk=ap_id AND EXISTS (SELECT * FROM ach_player_objective WHERE apo_objective=ao_id AND apo_state='GRANT' AND apo_player='".$elem['amc_character']."'))","ARRAY");
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$DBc->sendSQL("INSERT INTO ach_player_perk (app_objective,app_player,app_date) VALUES ('".$res[$i]['ap_id']."','".$elem['amc_character']."','".time()."')","NONE");
			}
		}
	}

	if($CONF['sleep_time'] != false) {
		sleep($CONF['sleep_time']);
	}

	//self call if cron mode is on
	if($MODE == "CRON" && $CONF['enable_selfcall'] == true) {
		$DBc->sendSQL("UPDATE ach_monitor_state SET ams_end='".time()."' WHERE ams_id='".$RID."'","NONE");

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

	if($logfile) {
		$logfile->write();
	}

	exit(0);
?>