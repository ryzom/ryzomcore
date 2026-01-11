<?php
	header('Content-type: text/xml');

	error_reporting(E_ALL ^ E_NOTICE);
	ini_set("display_errors","1");

	require_once("class/mySQL_class.php");
	require_once("conf.php");

		//create database connection
	$DBc = new mySQL($_CONF['mysql_error']);
	$DBc->connect($_CONF['mysql_server'],$_CONF['mysql_user'],$_CONF['mysql_pass'],$_CONF['mysql_database']);

	echo '<?xml version="1.0" ?><ryzom_progress>';
	echo "<character id='".$_REQUEST['cid']."' />";

	echo "<tasks>";
	$res = $DBc->sendSQL("SELECT * FROM ach_player_task WHERE apt_player='".$DBc->mre($_REQUEST['cid'])."'","ARRAY");
	foreach($res as $elem) {
		echo "<task id='".$elem['apt_task']."' date='".$elem['apt_date']."' />";
	}
	echo "</tasks>";

	echo "<objectives>";
	echo "<done>";
	$res = $DBc->sendSQL("SELECT * FROM ach_player_objective WHERE apo_player='".$DBc->mre($_REQUEST['cid'])."'","ARRAY");
	foreach($res as $elem) {
		echo "<objective id='".$elem['apo_objective']."' date='".$elem['apo_date']."' />";
	}
	echo "</done><open>";
	$res = $DBc->sendSQL("SELECT ao_id,(SELECT count(*) FROM ach_player_atom,ach_atom WHERE apa_player='".$DBc->mre($_REQUEST['cid'])."' AND atom_id=apa_atom AND atom_objective=ao_id) as anz FROM ach_objective WHERE ao_display='value' AND NOT EXISTS (SELECT * FROM ach_player_objective WHERE apo_player='".$DBc->mre($_REQUEST['cid'])."' AND apo_objective='ao_id') AND EXISTS (SELECT * FROM ach_player_atom,ach_atom WHERE apa_player='".$DBc->mre($_REQUEST['cid'])."' AND atom_id=apa_atom AND atom_objective=ao_id)","ARRAY");
	foreach($res as $elem) {
		echo "<objective id='".$elem['ao_id']."' value='".$elem['anz']."' />";
	}
	echo "</open>";
	echo "</objectives>";

	echo "</ryzom_progress>";

	die();
?>
