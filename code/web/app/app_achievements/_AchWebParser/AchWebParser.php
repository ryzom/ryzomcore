<?php
	error_reporting(E_ALL ^ E_NOTICE);
	ini_set("display_errors","1");

	if(file_exists("parser.stop")) {
		exit(0);
	}

	require_once("class/mySQL_class.php");
	require_once("conf.php");
	require_once("include/functions_inc.php");
	require_once("class/Entity_abstract.php");

	if(!$_REQUEST['file']) {
		$_REQUEST['file'] = $argv[1];
	}

	$log = new Logfile();
	if($CONF['logging'] == true) {
		require_once("class/Logfile_class.php");
		$log = new Logfile($CONF['logfile']);
	}

	$log->logf("File: '".$_REQUEST['file']."'");

	$log->logf("Starting up... ",false);

	if(!$_REQUEST['file']) {
		$log->logf("ERROR: no file given! EXITING!");
		die();
	}

	//create database connection
	$DBc = new mySQL($CONF['mysql_error']);
	$DBc->connect($CONF['mysql_server'],$CONF['mysql_user'],$CONF['mysql_pass'],$CONF['mysql_database']);

	$DBc_char = new mySQL($_CONF['mysql_error']);
	$DBc_char->connect($CONF['char_mysql_server'],$CONF['char_mysql_user'],$CONF['char_mysql_pass'],$CONF['char_mysql_database']);


	require_once("class/DataDispatcher_class.php");
	require_once("class/DataSourceHandler_class.php");
	require_once("class/SourceDriver_abstract.php");
	require_once("class/Callback_class.php");
	require_once("class/ValueCache_class.php");
	require_once("class/Atom_class.php");
	
	$_CACHE = new ValueCache();

	//create datasource handler
	$_DISPATCHER = new DataDispatcher();
	$_DATASOURCE = new DataSourceHandler();
	foreach($CONF['data_source'] as $elem) { //populate
		require_once("source/".$elem."/".$elem."_class.php");
		eval('$tmp = new '.$elem.'();');
		$_DATASOURCE->addSource($tmp);
	}

	#REPLACE WITH REGEX!!!

	$tmp = explode("/",$_REQUEST['file']);
	$tmp2 = explode("_",$tmp[(sizeof($tmp)-1)]);

	
	$chars = array(($tmp2[1]*16+$tmp2[2]));

	$log->logf(" done!");
	
	foreach($chars as $cid) {
		#STEP 1: load and register atoms

		$log->logf("Processing char '".$cid."' ...");

		$log->logi("Loading and registering Atoms... ",false);

		$atom_list = array();

		$res = $DBc->sendSQL("SELECT ap_id FROM ach_perk WHERE NOT EXISTS (SELECT * FROM ach_player_perk WHERE app_player='".$cid."' AND app_perk=ap_id)","ARRAY");
		foreach($res as $perk) {
			//get unfinished atoms belonging to unfinished objectives
			$res2 = $DBc->sendSQL("SELECT ach_atom.* FROM ach_atom,ach_objective,ach_achievement WHERE ao_perk='".$perk['ap_id']."' AND ao_id=atom_objective AND ap_achievement=aa_id AND NOT EXISTS (SELECT * FROM ach_player_objective WHERE apo_player='".$cid."' AND apo_objective=ao_id) AND (aa_tie_race IS NULL OR aa_tie_race='') AND (aa_tie_cult IS NULL OR aa_tie_cult='') AND (aa_tie_civ IS NULL OR aa_tie_civ='')","ARRAY");
			foreach($res2 as $atom) {
				$a = new Atom($atom,$cid);
				$atom_list[] = $a;
				$a->register();
			}
		}

		$log->logf("done!");

		$log->logi("Driving data... ",false);
		
		#STEP 2: drive data
		$_CACHE->setChar($cid);
		$_DATASOURCE->drive($cid);

		$log->logf("done!");

		#STEP 3: detect obj/perk progression
		$log->logi("Detecting Objectives... ",false);
		//obj
		$res = $DBc->sendSQL("SELECT ao_id FROM ach_objective WHERE ao_condition='all' AND NOT EXISTS (SELECT * FROM ach_player_objective WHERE apo_objective=ao_id AND apo_player='".$cid."') AND NOT EXISTS (SELECT * FROM ach_atom WHERE atom_objective=ao_id AND NOT EXISTS (SELECT * FROM ach_player_atom WHERE apa_atom=atom_id AND apa_state='GRANT' AND apa_player='".$cid."')) AND EXISTS (SELECT * FROM ach_atom WHERE atom_objective=ao_id)","ARRAY");
		$sz = sizeof($res);
		for($i=0;$i<$sz;$i++) {
			$DBc->sendSQL("INSERT INTO ach_player_objective (apo_objective,apo_player,apo_date) VALUES ('".$res[$i]['ao_id']."','".$cid."','".time()."')","NONE");
		}

		$res = $DBc->sendSQL("SELECT ao_id FROM ach_objective WHERE ao_condition='value' AND NOT EXISTS (SELECT * FROM ach_player_objective WHERE apo_objective=ao_id AND apo_player='".$cid."') AND ao_value<=(SELECT sum(apa_value) FROM ach_atom,ach_player_atom WHERE atom_objective=ao_id AND apa_atom=atom_id AND apa_state='GRANT' AND apa_player='".$cid."') AND EXISTS (SELECT * FROM ach_atom WHERE atom_objective=ao_id)","ARRAY");
		$sz = sizeof($res);
		for($i=0;$i<$sz;$i++) {
			$DBc->sendSQL("INSERT INTO ach_player_objective (apo_objective,apo_player,apo_date) VALUES ('".$res[$i]['ao_id']."','".$cid."','".time()."')","NONE");
		}

		$res = $DBc->sendSQL("SELECT ao_id FROM ach_objective WHERE ao_condition='any' AND NOT EXISTS (SELECT * FROM ach_player_objective WHERE apo_objective=ao_id AND apo_player='".$cid."') AND EXISTS (SELECT * FROM ach_atom WHERE atom_objective=ao_id AND EXISTS (SELECT * FROM ach_player_atom WHERE apa_atom=atom_id AND apa_state='GRANT' AND apa_player='".$cid."')) AND EXISTS (SELECT * FROM ach_atom WHERE atom_objective=ao_id)","ARRAY");
		$sz = sizeof($res);
		for($i=0;$i<$sz;$i++) {
			$DBc->sendSQL("INSERT INTO ach_player_objective (apo_objective,apo_player,apo_date) VALUES ('".$res[$i]['ao_id']."','".$cid."','".time()."')","NONE");
		}

		$log->logf("done!");
		$log->logi("Clearing atom data... ",false);
		
		//clear atom state for completed objectives
		$DBc->sendSQL("DELETE FROM ach_player_atom WHERE EXISTS (SELECT * FROM ach_player_objective,ach_atom WHERE atom_id=apa_atom AND apa_player='".$cid."' AND atom_objective=ao_id)");

		$log->logf("done!");

		//perk
		$log->logi("Detecting Perks... ",false);
		$res = $DBc->sendSQL("SELECT ap_id FROM ach_perk WHERE ap_condition='all' AND NOT EXISTS (SELECT * FROM ach_player_perk WHERE app_perk=ap_id AND app_player='".$cid."') AND NOT EXISTS (SELECT * FROM ach_objective WHERE ao_perk=ap_id AND NOT EXISTS (SELECT * FROM ach_player_objective WHERE apo_objective=ao_id AND apo_player='".$cid."')) AND EXISTS (SELECT * FROM ach_objective WHERE ao_perk=ap_id) AND ap_dev='0'","ARRAY");
		$sz = sizeof($res);
		for($i=0;$i<$sz;$i++) {
			$DBc->sendSQL("INSERT INTO ach_player_perk (app_perk,app_player,app_date) VALUES ('".$res[$i]['ap_id']."','".$cid."','".time()."')","NONE");
		}

		$res = $DBc->sendSQL("SELECT ap_id FROM ach_perk WHERE ap_condition='value' AND NOT EXISTS (SELECT * FROM ach_player_perk WHERE app_perk=ap_id AND app_player='".$cid."') AND ap_value<=(SELECT count(*) FROM ach_objective WHERE ao_perk=ap_id AND EXISTS (SELECT * FROM ach_player_objective WHERE apo_objective=ao_id AND apo_player='".$cid."')) AND EXISTS (SELECT * FROM ach_objective WHERE ao_perk=ap_id) AND ap_dev='0'","ARRAY");
		$sz = sizeof($res);
		for($i=0;$i<$sz;$i++) {
			$DBc->sendSQL("INSERT INTO ach_player_perk (app_perk,app_player,app_date) VALUES ('".$res[$i]['ap_id']."','".$cid."','".time()."')","NONE");
		}

		$res = $DBc->sendSQL("SELECT ap_id FROM ach_perk WHERE ap_condition='any' AND NOT EXISTS (SELECT * FROM ach_player_perk WHERE app_perk=ap_id AND app_player='".$cid."') AND EXISTS (SELECT * FROM ach_objective WHERE ao_perk=ap_id AND EXISTS (SELECT * FROM ach_player_objective WHERE apo_objective=ao_id AND apo_player='".$cid."')) AND EXISTS (SELECT * FROM ach_objective WHERE ao_perk=ap_id) AND ap_dev='0'","ARRAY");
		$sz = sizeof($res);
		for($i=0;$i<$sz;$i++) {
			$DBc->sendSQL("INSERT INTO ach_player_perk (app_perk,app_player,app_date) VALUES ('".$res[$i]['ap_id']."','".$cid."','".time()."')","NONE");
		}

		$log->logf("done!");
	}
	
	$log->logf("Run complete; exiting...");
	exit(0);
?>