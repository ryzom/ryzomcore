<?php
	error_reporting(E_ALL ^ E_NOTICE);
	ini_set("display_errors","1");

	$tmp_log_xmlgen_time = 0;

	$microstart = explode(' ',microtime());
	$start_time = $microstart[0] + $microstart[1];

	$MY_PATH = dirname(__FILE__);

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

	if($CONF['logging'] == true) {
		require_once("class/Logfile_class.php");
		$log = new Logfile($CONF['logfile']);
	}

	$log->logf("File: '".$_REQUEST['file']."'");

	$log->logf("Starting up... ",false);

	if(!$_REQUEST['file']) {
		$log->logf("ERROR: no file given! EXITING!");
		$log->close();
		die();
	}

	//create database connection
	$DBc = new mySQL($CONF['mysql_error']);
	$DBc->connect($CONF['mysql_server'],$CONF['mysql_user'],$CONF['mysql_pass'],$CONF['mysql_database']);


	require_once("class/DataDispatcher_class.php");
	require_once("class/DataSourceHandler_class.php");
	require_once("class/SourceDriver_abstract.php");
	require_once("class/Callback_class.php");
	require_once("class/ValueCache_class.php");
	require_once("class/Atom_class.php");
	require_once("class/XMLgenerator_class.php");
	require_once("class/XMLnode_class.php");
	require_once("class/XMLfile_class.php");
	require_once("class/Stats_class.php");
	
	$_CACHE = new ValueCache();

	//new generator for API xml files.
	$XMLgenerator = new XMLgenerator();

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

	$DBc->database($CONF['char_mysql_database']);

	$res = $DBc->sendSQL("SELECT id FROM players WHERE cid='".$DBc->mre($tmp2[1]*16+$tmp2[2])."' AND deleted='0'","ARRAY");
	#$res[0]['id'] = 1;
	if(($res[0]['id'] > 0) == false) {
		$log->logf("ERROR: no character found!");
		$log->close();
		die();
	}
	$cdata = array("cid"=>$res[0]['id'],"aid"=>$tmp2[1],"sid"=>$tmp2[2]);

	#$cdata = array("cid"=>1,"aid"=>1,"sid"=>1);

	$DBc->database($CONF['mysql_database']);

#echo var_export($cdata);

	$log->logf(" done!");

	$microstop = explode(' ',microtime());
	$stop_time = $microstop[0] + $microstop[1];

	$log->logf("Expired time: ".($stop_time - $start_time));
	
	#foreach($chars as $cid) {
		#STEP 1: load and register atoms

		$log->logf("Processing char '".$cdata['cid']."' ...");

		$log->logi("Loading and registering Atoms... ",false);

		$atom_list = array();

		$res = $DBc->sendSQL("SELECT at_id FROM ach_task WHERE NOT EXISTS (SELECT * FROM ach_player_task WHERE apt_player='".$cdata['cid']."' AND apt_task=at_id) AND at_dev='0'","ARRAY");
		foreach($res as $task) {
			//get unfinished atoms belonging to unfinished objectives
			$res2 = $DBc->sendSQL("SELECT ach_atom.* FROM ach_atom,ach_objective WHERE ao_task='".$task['at_id']."' AND ao_id=atom_objective AND NOT EXISTS (SELECT * FROM ach_player_objective WHERE apo_player='".$cdata['cid']."' AND apo_objective=ao_id)","ARRAY");
			foreach($res2 as $atom) {
				$a = new Atom($atom,$cdata);
				$atom_list[] = $a;
				$atom_list[] = $a;
				$a->register();
			}
		}

		$statsdb = new Stats();
		$statsdb->register();

		$log->logf("done!");

		$log->logf("Memory load: ".memory_get_usage()." bytes");
		$microstop = explode(' ',microtime());
		$stop_time = $microstop[0] + $microstop[1];

		$log->logf("Expired time: ".($stop_time - $start_time));

		$log->logi("Driving data... ",false);
		
		#STEP 2: drive data
		$_CACHE->setChar($cdata);
		$res = $DBc->sendSQL("SELECT sum(at_value) as anz FROM ach_task,ach_player_task WHERE at_id=apt_task AND apt_player='".$cdata['cid']."'","ARRAY");
		$_DISPATCHER->dispatchValue("yubopoints",$res[0]['anz']);

		$_DISPATCHER->dispatchValue("aid",$cdata['aid']);
		$_DISPATCHER->dispatchValue("cid",$cdata['cid']);
		$_DISPATCHER->dispatchValue("sid",$cdata['sid']);
		$_DATASOURCE->drive($cdata);

		//save daily stats
		$res = $DBc->sendSQL("SELECT COUNT(*) as anz FROM stat_daily WHERE sdm_day='".date("Y-m-d",time())."'","ARRAY");
		if($res[0]['anz'] == 0) {
			$res = $DBc->sendSQL("SELECT SUM(sp_money) as all_money, AVG(sp_money) as avg_money, COUNT(*) as playercount FROM app_achievements.stat_players as s, webig.players as p, ring_live.characters as c, nel.user as n WHERE s.sp_char = p.id AND p.cid = c.char_id AND c.user_id = n.uid AND n.privilege=''","ARRAY");

			$res2 = $DBc->sendSQL("SELECT sp_money FROM app_achievements.stat_players as s, webig.players as p, ring_live.characters as c, nel.user as n WHERE s.sp_char = p.id AND p.cid = c.char_id AND c.user_id = n.uid AND n.privilege='' ORDER by sp_money ASC LIMIT ".floor($res[0]['playercount']/2).",1","ARRAY");

			$res3 = $DBc->sendSQL("SELECT SUM(sp_yubototal) as all_yubo, AVG(sp_yubototal) as avg_yubo FROM app_achievements.stat_players as s, webig.players as p, ring_live.characters as c, nel.user as n WHERE s.sp_char = p.id AND p.cid = c.char_id AND c.user_id = n.uid AND n.privilege=''","ARRAY");

			$res4 = $DBc->sendSQL("SELECT sp_yubototal FROM app_achievements.stat_players as s, webig.players as p, ring_live.characters as c, nel.user as n WHERE s.sp_char = p.id AND p.cid = c.char_id AND c.user_id = n.uid AND n.privilege='' ORDER by sp_money ASC LIMIT ".floor($res[0]['playercount']/2).",1","ARRAY");

			$res5 = $DBc->sendSQL("SELECT SUM(sp_mekcount) as all_mek, AVG(sp_mekcount) as avg_mek FROM app_achievements.stat_players as s, webig.players as p, ring_live.characters as c, nel.user as n WHERE s.sp_char = p.id AND p.cid = c.char_id AND c.user_id = n.uid AND n.privilege=''","ARRAY");

			$res6 = $DBc->sendSQL("SELECT sp_mekcount FROM app_achievements.stat_players as s, webig.players as p, ring_live.characters as c, nel.user as n WHERE s.sp_char = p.id AND p.cid = c.char_id AND c.user_id = n.uid AND n.privilege='' ORDER by sp_money ASC LIMIT ".floor($res[0]['playercount']/2).",1","ARRAY");

			$res7 = $DBc->sendSQL("SELECT SUM(sp_maxlevel) as all_lvl, AVG(sp_maxlevel) as avg_lvl FROM app_achievements.stat_players as s, webig.players as p, ring_live.characters as c, nel.user as n WHERE s.sp_char = p.id AND p.cid = c.char_id AND c.user_id = n.uid AND n.privilege=''","ARRAY");

			$res8 = $DBc->sendSQL("SELECT sp_maxlevel FROM app_achievements.stat_players as s, webig.players as p, ring_live.characters as c, nel.user as n WHERE s.sp_char = p.id AND p.cid = c.char_id AND c.user_id = n.uid AND n.privilege='' ORDER by sp_money ASC LIMIT ".floor($res[0]['playercount']/2).",1","ARRAY");

			$res9 = $DBc->sendSQL("SELECT SUM(sp_itemcount) as all_item, AVG(sp_itemcount) as avg_item FROM app_achievements.stat_players as s, webig.players as p, ring_live.characters as c, nel.user as n WHERE s.sp_char = p.id AND p.cid = c.char_id AND c.user_id = n.uid AND n.privilege=''","ARRAY");

			$res10 = $DBc->sendSQL("SELECT sp_itemcount FROM app_achievements.stat_players as s, webig.players as p, ring_live.characters as c, nel.user as n WHERE s.sp_char = p.id AND p.cid = c.char_id AND c.user_id = n.uid AND n.privilege='' ORDER by sp_money ASC LIMIT ".floor($res[0]['playercount']/2).",1","ARRAY");

			$DBc->sendSQL("INSERT IGNORE INTO stat_daily (sd_day,sd_players,sd_money_avg,sd_money_total,sd_money_mean,sd_mek_total,sd_mek_avg,sd_mek_mean,sd_yubo_total,sd_yubo_avg,sd_yubo_mean,sd_lvl_total,sd_lvl_avg,sd_lvl_mean,sd_item_total,sd_item_avg,sd_item_mean) VALUES ('".date("Y-m-d",time())."','".$res[0]['playercount']."','".$res[0]['avg_money']."','".$res[0]['all_money']."','".$res2[0]['sp_money']."','".$res5[0]['all_mek']."','".$res5[0]['avg_mek']."','".$res6[0]['sp_mekcount']."','".$res3[0]['all_yubo']."','".$res3[0]['avg_yubo']."','".$res4[0]['sp_yubototal']."','".$res7[0]['all_lvl']."','".$res7[0]['avg_lvl']."','".$res8[0]['sp_maxlevel']."','".$res9[0]['all_item']."','".$res9[0]['avg_item']."','".$res102[0]['sp_itemcounty']."')","NONE");
		}

		$log->logf("done!");
		$microstop = explode(' ',microtime());
		$stop_time = $microstop[0] + $microstop[1];

		$log->logf("Expired time: ".($stop_time - $start_time));
		$log->logf("xml-gen took: ".$tmp_log_xmlgen_time);
		$log->logf("Memory load: ".memory_get_usage()." bytes");

		$log->logi("Writing XML export... ",false);

		$XMLgenerator->generate();

		$log->logf("done!");

		$log->logf("Memory load: ".memory_get_usage()." bytes");
		$microstop = explode(' ',microtime());
		$stop_time = $microstop[0] + $microstop[1];

		$log->logf("Expired time: ".($stop_time - $start_time));

		#STEP 3: detect obj/task progression
		$log->logi("Detecting Objectives... ",false);

		//obj
		$res = $DBc->sendSQL("SELECT ao_id FROM ach_objective WHERE ao_condition='all' AND NOT EXISTS (SELECT * FROM ach_player_objective WHERE apo_objective=ao_id AND apo_player='".$cdata['cid']."') AND NOT EXISTS (SELECT * FROM ach_atom WHERE atom_objective=ao_id AND NOT EXISTS (SELECT * FROM ach_player_atom WHERE apa_atom=atom_id AND apa_state='GRANT' AND apa_player='".$cdata['cid']."')) AND EXISTS (SELECT * FROM ach_atom WHERE atom_objective=ao_id)","ARRAY");
		$sz = sizeof($res);
		for($i=0;$i<$sz;$i++) {
			$DBc->sendSQL("INSERT INTO ach_player_objective (apo_objective,apo_player,apo_date) VALUES ('".$res[$i]['ao_id']."','".$cdata['cid']."','".time()."')","NONE");
		}

		$log->logf("1... ",false);

		$res = $DBc->sendSQL("SELECT ao_id FROM ach_objective WHERE ao_condition='value' AND NOT EXISTS (SELECT * FROM ach_player_objective WHERE apo_objective=ao_id AND apo_player='".$cdata['cid']."') AND ao_value<=(SELECT sum(apa_value) FROM ach_atom,ach_player_atom WHERE atom_objective=ao_id AND apa_atom=atom_id AND apa_state='GRANT' AND apa_player='".$cdata['cid']."') AND EXISTS (SELECT * FROM ach_atom WHERE atom_objective=ao_id)","ARRAY");
		$sz = sizeof($res);
		for($i=0;$i<$sz;$i++) {
			$DBc->sendSQL("INSERT INTO ach_player_objective (apo_objective,apo_player,apo_date) VALUES ('".$res[$i]['ao_id']."','".$cdata['cid']."','".time()."')","NONE");
		}

		$log->logf("2... ",false);

		$res = $DBc->sendSQL("SELECT ao_id FROM ach_objective WHERE ao_condition='any' AND NOT EXISTS (SELECT * FROM ach_player_objective WHERE apo_objective=ao_id AND apo_player='".$cdata['cid']."') AND EXISTS (SELECT * FROM ach_atom WHERE atom_objective=ao_id AND EXISTS (SELECT * FROM ach_player_atom WHERE apa_atom=atom_id AND apa_state='GRANT' AND apa_player='".$cdata['cid']."')) AND EXISTS (SELECT * FROM ach_atom WHERE atom_objective=ao_id)","ARRAY");
		$sz = sizeof($res);
		for($i=0;$i<$sz;$i++) {
			$DBc->sendSQL("INSERT INTO ach_player_objective (apo_objective,apo_player,apo_date) VALUES ('".$res[$i]['ao_id']."','".$cdata['cid']."','".time()."')","NONE");
		}

		$log->logf("3... ",false);

		//meta
		#$res = $DBc->sendSQL("SELECT ao_id FROM ach_objective,ach_task as t1,ach_achievement WHERE ao_display='meta' AND ao_task=t1.at_id AND t1.at_achievement=aa_id AND NOT EXISTS (SELECT * FROM ach_task as t2 WHERE t2.at_achievement=ao_metalink AND NOT EXISTS (SELECT * FROM ach_player_task WHERE apt_task=t2.at_id AND apt_player='".$cdata['cid']."'))","ARRAY");
		$res = $DBc->sendSQL("SELECT ao_id FROM ach_objective WHERE ao_display='meta' AND NOT EXISTS (SELECT * FROM ach_player_objective WHERE apo_objective=ao_id AND apo_player='".$cdata['cid']."') AND NOT EXISTS (SELECT * FROM ach_task WHERE ao_metalink=at_achievement AND NOT EXISTS (SELECT * FROM ach_player_task WHERE apt_task=at_id AND apt_player='".$cdata['cid']."'))","ARRAY");
		$sz = sizeof($res);
		for($i=0;$i<$sz;$i++) {
			$DBc->sendSQL("INSERT INTO ach_player_objective (apo_objective,apo_player,apo_date) VALUES ('".$res[$i]['ao_id']."','".$cdata['cid']."','".time()."')","NONE");
		}

		$log->logf("4... ",false);

		$log->logf("done!");

		$microstop = explode(' ',microtime());
		$stop_time = $microstop[0] + $microstop[1];

		$log->logf("Expired time: ".($stop_time - $start_time));

		$log->logi("Clearing atom data... ",false);
		
		//clear atom state for completed objectives
		#$DBc->sendSQL("DELETE FROM ach_player_atom WHERE EXISTS (SELECT * FROM ach_player_objective,ach_atom WHERE atom_id=apa_atom AND apa_player='".$cdata['cid']."' AND atom_objective=apo_objective)");

		$DBc->sendSQL("DELETE FROM ach_player_atom WHERE apa_player='".$cdata['cid']."' AND EXISTS (SELECT * FROM ach_player_objective,ach_atom WHERE atom_id=apa_atom AND apo_player='".$cdata['cid']."' AND atom_objective=apo_objective)");

		$log->logf("done!");

		$microstop = explode(' ',microtime());
		$stop_time = $microstop[0] + $microstop[1];

		$log->logf("Expired time: ".($stop_time - $start_time));

		//task
		$log->logi("Detecting Tasks... ",false);
		$res = $DBc->sendSQL("SELECT at_id FROM ach_task WHERE at_condition='all' AND NOT EXISTS (SELECT * FROM ach_player_task WHERE apt_task=at_id AND apt_player='".$cdata['cid']."') AND NOT EXISTS (SELECT * FROM ach_objective WHERE ao_task=at_id AND NOT EXISTS (SELECT * FROM ach_player_objective WHERE apo_objective=ao_id AND apo_player='".$cdata['cid']."')) AND EXISTS (SELECT * FROM ach_objective WHERE ao_task=at_id) AND at_dev='0'","ARRAY");
		$sz = sizeof($res);
		for($i=0;$i<$sz;$i++) {
			$DBc->sendSQL("INSERT INTO ach_player_task (apt_task,apt_player,apt_date) VALUES ('".$res[$i]['at_id']."','".$cdata['cid']."','".time()."')","NONE");
		}

$log->logf("1... ",false);

		$res = $DBc->sendSQL("SELECT at_id FROM ach_task WHERE at_condition='value' AND NOT EXISTS (SELECT * FROM ach_player_task WHERE apt_task=at_id AND apt_player='".$cdata['cid']."') AND at_value<=(SELECT count(*) FROM ach_objective WHERE ao_task=at_id AND EXISTS (SELECT * FROM ach_player_objective WHERE apo_objective=ao_id AND apo_player='".$cdata['cid']."')) AND EXISTS (SELECT * FROM ach_objective WHERE ao_task=at_id) AND at_dev='0'","ARRAY");
		$sz = sizeof($res);
		for($i=0;$i<$sz;$i++) {
			$DBc->sendSQL("INSERT INTO ach_player_task (apt_task,apt_player,apt_date) VALUES ('".$res[$i]['at_id']."','".$cdata['cid']."','".time()."')","NONE");
		}

$log->logf("2... ",false);

		$res = $DBc->sendSQL("SELECT at_id FROM ach_task WHERE at_condition='any' AND NOT EXISTS (SELECT * FROM ach_player_task WHERE apt_task=at_id AND apt_player='".$cdata['cid']."') AND EXISTS (SELECT * FROM ach_objective WHERE ao_task=at_id AND EXISTS (SELECT * FROM ach_player_objective WHERE apo_objective=ao_id AND apo_player='".$cdata['cid']."')) AND EXISTS (SELECT * FROM ach_objective WHERE ao_task=at_id) AND at_dev='0'","ARRAY");
		$sz = sizeof($res);
		for($i=0;$i<$sz;$i++) {
			$DBc->sendSQL("INSERT INTO ach_player_task (apt_task,apt_player,apt_date) VALUES ('".$res[$i]['at_id']."','".$cdata['cid']."','".time()."')","NONE");
		}

$log->logf("3... ",false);

		/*if($CONF['facebook'] == true) {
			require_once("../fb/facebook.php");

			$facebook = new Facebook(array(appId=>$CONF['fb_id'], secret=>$CONF['fb_secret']));


			$res = $DBc->sendSQL("SELECT * FROM ach_fb_token WHERE aft_player='".$cdata['cid']."'","ARRAY");

			$access_token = $res[0]['aft_token'];

			if($res[0]['aft_allow'] == 1) {

				$res2 = $DBc->sendSQL("SELECT * FROM ach_player_task WHERE apt_player='".$cdata['cid']."' AND apt_fb='0'","ARRAY");
				$sz = sizeof($res2);

				for($i=0;$i<$sz;$i++) {

					//this has to be adapted!

					#$result = $facebook->api(
					#	'/me/feed/',
					#	array('access_token' => $$access_token, 'message' => 'Playing around with FB Graph..')
					#);

				}

				$DBc->sendSQL("UPDATE ach_player_task SET apt_fb='1' WHERE apt_player='".$cdata['cid']."'","NONE");




			}
		}*/

		$log->logf("done!");
	#}

	$microstop = explode(' ',microtime());
	$stop_time = $microstop[0] + $microstop[1];

	$log->logf("Total time: ".($stop_time - $start_time));
	
	$log->logf("Run complete; exiting...");
	$log->close();
	exit(0);
?>