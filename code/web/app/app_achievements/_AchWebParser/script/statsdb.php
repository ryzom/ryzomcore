<?php
	$this->registerValue("_money","_statsdb_money");
	function _statsdb_money($money,$_P,$_CB) {
		global $cdata,$DBc;
		$_IDENT = "_money";

		$DBc->sendSQL("INSERT INTO stat_players (sp_char,sp_money) VALUES ('".$cdata['cid']."','".$money."') ON DUPLICATE KEY UPDATE sp_money='".$money."'","NONE");

		$_P->unregisterValue($_IDENT,$_CB);
	}

	$this->registerValue("_race","_statsdb_race");
	function _statsdb_race($race,$_P,$_CB) {
		global $cdata,$DBc;
		$_IDENT = "_race";

		$race = "r_".strtolower($race);

		$DBc->sendSQL("INSERT INTO stat_players (sp_char,sp_race) VALUES ('".$cdata['cid']."','".$race."') ON DUPLICATE KEY UPDATE sp_race='".$race."'","NONE");

		$_P->unregisterValue($_IDENT,$_CB);
	}

	$this->registerValue("yubopoints","_statsdb_yubototal");
	function _statsdb_yubototal($yubo,$_P,$_CB) {
		global $cdata,$DBc;
		$_IDENT = "yubopoints";

		$DBc->sendSQL("INSERT INTO stat_players (sp_char,sp_yubototal) VALUES ('".$cdata['cid']."','".$yubo."') ON DUPLICATE KEY UPDATE sp_yubototal='".$yubo."'","NONE");

		$_P->unregisterValue($_IDENT,$_CB);
	}

	$this->registerValue("petcount","_statsdb_mekcount");
	function _statsdb_mekcount($count,$_P,$_CB) {
		global $cdata,$DBc;
		$_IDENT = "petcount";

		$DBc->sendSQL("INSERT INTO stat_players (sp_char,sp_mekcount) VALUES ('".$cdata['cid']."','".$count."') ON DUPLICATE KEY UPDATE sp_mekcount='".$count."'","NONE");

		$_P->unregisterValue($_IDENT,$_CB);
	}

	$this->registerValue("skilllist","_statsdb_maxlevel");
	function _statsdb_maxlevel($skills,$_P,$_CB) {
		global $cdata,$DBc,$log;
		$_IDENT = "skilllist";

		$log->logf("rcv skilllist: ".var_export($skills,true));

		$lvl = 0;
		foreach($skills->skills as $elem) {
			if($elem->current > $lvl) {
				$lvl = $elem->current;
			}
		}

		$DBc->sendSQL("INSERT INTO stat_players (sp_char,sp_maxlevel) VALUES ('".$cdata['cid']."','".$lvl."') ON DUPLICATE KEY UPDATE sp_maxlevel='".$lvl."'","NONE");

		$_P->unregisterValue($_IDENT,$_CB);
	}

	$this->registerValue("_guildid","_statsdb_guildid");
	function _statsdb_guildid($id,$_P,$_CB) {
		global $cdata,$DBc;
		$_IDENT = "_guildid";

		$DBc->sendSQL("INSERT INTO stat_players (sp_char,sp_guildid) VALUES ('".$cdata['cid']."','".$id."') ON DUPLICATE KEY UPDATE sp_guildid='".$id."'","NONE");

		$_P->unregisterValue($_IDENT,$_CB);
	}

	$this->registerValue("itemcount","_statsdb_itemcount");
	function _statsdb_itemcount($count,$_P,$_CB) {
		global $cdata,$DBc;
		$_IDENT = "itemcount";

		$DBc->sendSQL("INSERT INTO stat_players (sp_char,sp_itemcount) VALUES ('".$cdata['cid']."','".$count."') ON DUPLICATE KEY UPDATE sp_itemcount='".$count."'","NONE");

		$_P->unregisterValue($_IDENT,$_CB);
	}
?>