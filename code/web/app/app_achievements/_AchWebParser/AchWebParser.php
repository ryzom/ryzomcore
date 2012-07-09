<?php
	error_reporting(E_ALL ^ E_NOTICE);
	ini_set("display_errors","1");

	if(file_exists("parser.stop")) {
		exit(0);
	}

	require_once("class/mySQL_class.php");
	require_once("conf.php");
	require_once("include/functions_inc.php");

	$_REQUEST['file'] = $argv[1];


	/*$logfile = false;
	if($CONF['logging'] == true) {
		require_once("class/Logfile_class.php");
		#$logfile = fopen($CONF['logfile'].'.'.date("Ymd",time()).'.txt','a');
		$logfile = new Logfile($CONF['logfile']);#!! MUST HAVE ONE LOGFILE PER RUN!!
	}*/

	//set mode: cron || single with given cid
	#MISSING: conf to allow external calls; whitelist ips
	/*$MODE = "CRON";
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
	}*/

	//create database connection
	$DBc = new mySQL($CONF['mysql_error']);
	$DBc->connect($CONF['mysql_server'],$CONF['mysql_user'],$CONF['mysql_pass'],$CONF['mysql_database']);

	#if($MODE == "CRON") {
	#	$RID = $DBc->sendSQL("INSERT INTO ach_monitor_state (ams_start,ams_end) VALUES ('".time()."','0')","INSERT"); // insert run into monitoring table
	#}

	require_once("class/DataDispatcher_class.php");
	require_once("class/DataSourceHandler_class.php");
	require_once("class/SourceDriver_abstract.php");
	require_once("class/Callback_class.php");
	require_once("class/ValueCache_class.php");
	require_once("class/Atom_class.php");
	
	#MISSING: static ValueCache!

	$_CACHE = new ValueCache();


	//create datasource handler
	$_DISPATCHER = new DataDispatcher();
	$_DATASOURCE = new DataSourceHandler();
	foreach($CONF['data_source'] as $elem) { //populate
		require_once("source/".$elem."/".$elem."_class.php");
		eval('$tmp = new '.$elem.'();');
		$_DATASOURCE->addSource($tmp);
	}

	// fetch candidates
	/*if($MODE == "SINGLE") {
		$chars = array();
		$chars[] = array('amc_character',$CID);
	}
	else {
		#$chars = array();

		$DBc->sendSQL("UPDATE ach_monitor_character SET amc_working='0' WHERE amc_last_import<'".(time()-60*60)."'"); // unlock if something went wrong

		$DBc->sendSQL("UPDATE ach_monitor_character SET amc_working='".$RID."' WHERE amc_last_login>amc_last_import AND amc_working='0'","NONE");
		
		$chars = $DBc->sendSQL("SELECT amc_character FROM ach_monitor_character WHERE amc_working='".$RID."'","ARRAY");
	}*/

	$tmp = explode("/",$_REQUEST['file']);
	$tmp2 = explode("_",$tmp[(sizeof($tmp)-1)]);

	
	$chars = array(($tmp2[1]*16+$tmp2[2]));
	
	foreach($chars as $cid) {
		#STEP 1: load and register atoms

		$atom_list = array();

		$res = $DBc->sendSQL("SELECT ap_id FROM ach_perk WHERE NOT EXISTS (SELECT * FROM ach_player_perk WHERE app_player='".$cid."' AND app_perk=ap_id)","ARRAY");
		foreach($res as $perk) {
			//get unfinished atoms belonging to unfinished objectives
			$res2 = $DBc->sendSQL("SELECT ach_atom.* FROM ach_atom,ach_objective WHERE ao_perk='".$perk['ap_id']."' AND ao_id=atom_objective AND NOT EXISTS (SELECT * FROM ach_player_objective WHERE apo_player='".$cid."' AND apo_objective=ao_id)","ARRAY");
			foreach($res2 as $atom) {
				$a = new Atom($atom,$cid);
				$atom_list[] = $a;
				$a->register();
			}
		}
		
		#STEP 2: drive data
		$_CACHE->setChar($cid);
		$_DATASOURCE->drive($cid);

		#STEP 3: detect obj/perk progression
		//obj
		$res = $DBc->sendSQL("SELECT ao_id FROM ach_objective WHERE ao_condition='all' AND NOT EXISTS (SELECT * FROM ach_player_objective WHERE apo_objective=ao_id AND apo_player='".$cid."') AND NOT EXISTS (SELECT * FROM ach_atom WHERE atom_objective=ao_id AND EXISTS (SELECT * FROM ach_player_atom WHERE apa_atom=atom_id AND apa_state!='GRANT' AND apa_player='".$cid."'))","ARRAY");
		$sz = sizeof($res);
		for($i=0;$i<$sz;$i++) {
			$DBc->sendSQL("INSERT INTO ach_player_objective (apo_objective,apo_player,apo_date) VALUES ('".$res[$i]['ao_id']."','".$cid."','".time()."')","NONE");
		}

		$res = $DBc->sendSQL("SELECT ao_id FROM ach_objective WHERE ao_condition='value' AND NOT EXISTS (SELECT * FROM ach_player_objective WHERE apo_objective=ao_id AND apo_player='".$cid."') AND ao_value<=(SELECT count(*) FROM ach_atom WHERE atom_objective=ao_id AND EXISTS (SELECT * FROM ach_player_atom WHERE apa_atom=atom_id AND apa_state='GRANT' AND apa_player='".$cid."'))","ARRAY");
		$sz = sizeof($res);
		for($i=0;$i<$sz;$i++) {
			$DBc->sendSQL("INSERT INTO ach_player_objective (apo_objective,apo_player,apo_date) VALUES ('".$res[$i]['ao_id']."','".$cid."','".time()."')","NONE");
		}

		$res = $DBc->sendSQL("SELECT ao_id FROM ach_objective WHERE ao_condition='any' AND NOT EXISTS (SELECT * FROM ach_player_objective WHERE apo_objective=ao_id AND apo_player='".$cid."') AND EXISTS (SELECT * FROM ach_atom WHERE atom_objective=ao_id AND EXISTS (SELECT * FROM ach_player_atom WHERE apa_atom=atom_id AND apa_state='GRANT' AND apa_player='".$cid."'))","ARRAY");
		$sz = sizeof($res);
		for($i=0;$i<$sz;$i++) {
			$DBc->sendSQL("INSERT INTO ach_player_objective (apo_objective,apo_player,apo_date) VALUES ('".$res[$i]['ao_id']."','".$cid."','".time()."')","NONE");
		}

		//perk
		$res = $DBc->sendSQL("SELECT ap_id FROM ach_perk WHERE ap_condition='all' AND NOT EXISTS (SELECT * FROM ach_player_perk WHERE app_perk=ap_id AND app_player='".$cid."') AND NOT EXISTS (SELECT * FROM ach_objective WHERE ao_perk=ap_id AND NOT EXISTS (SELECT * FROM ach_player_objective WHERE apo_objective=ao_id AND apo_player='".$cid."'))","ARRAY");
		$sz = sizeof($res);
		for($i=0;$i<$sz;$i++) {
			$DBc->sendSQL("INSERT INTO ach_player_perk (app_perk,app_player,app_date) VALUES ('".$res[$i]['ap_id']."','".$cid."','".time()."')","NONE");
		}

		$res = $DBc->sendSQL("SELECT ap_id FROM ach_perk WHERE ap_condition='value' AND NOT EXISTS (SELECT * FROM ach_player_perk WHERE app_perk=ap_id AND app_player='".$cid."') AND ap_value<=(SELECT count(*) FROM ach_objective WHERE ao_perk=ap_id AND EXISTS (SELECT * FROM ach_player_objective WHERE apo_objective=ao_id AND apo_player='".$cid."'))","ARRAY");
		$sz = sizeof($res);
		for($i=0;$i<$sz;$i++) {
			$DBc->sendSQL("INSERT INTO ach_player_perk (app_perk,app_player,app_date) VALUES ('".$res[$i]['ap_id']."','".$cid."','".time()."')","NONE");
		}

		$res = $DBc->sendSQL("SELECT ap_id FROM ach_perk WHERE ap_condition='any' AND NOT EXISTS (SELECT * FROM ach_player_perk WHERE app_perk=ap_id AND app_player='".$cid."') AND EXISTS (SELECT * FROM ach_objective WHERE ao_perk=ap_id AND EXISTS (SELECT * FROM ach_player_objective WHERE apo_objective=ao_id AND apo_player='".$cid."'))","ARRAY");
		$sz = sizeof($res);
		for($i=0;$i<$sz;$i++) {
			$DBc->sendSQL("INSERT INTO ach_player_perk (app_perk,app_player,app_date) VALUES ('".$res[$i]['ap_id']."','".$cid."','".time()."')","NONE");
		}
	}
	

	
?>