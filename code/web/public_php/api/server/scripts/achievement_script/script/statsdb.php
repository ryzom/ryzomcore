<?php
	$this->registerValue("_money","_statsdb_money");
	function _statsdb_money($money,$_P,$_CB) {
		global $cdata,$DBc,$statsdb;
		$_IDENT = "_money";

		#$DBc->sendSQL("UPDATE stat_players SET sp_money='".$money."' WHERE sp_char='".$cdata['cid']."'","NONE");
		$statsdb->setValue('sp_money',$money);

		$_P->unregisterValue($_IDENT,$_CB);
	}

	$this->registerValue("_race","_statsdb_race");
	function _statsdb_race($race,$_P,$_CB) {
		global $cdata,$DBc,$statsdb;
		$_IDENT = "_race";

		$race = "r_".strtolower($race);

		#$DBc->sendSQL("UPDATE stat_players SET sp_race='".$race."' WHERE sp_char='".$cdata['cid']."'","NONE");
		$statsdb->setValue('sp_race',$race);

		$_P->unregisterValue($_IDENT,$_CB);
	}

	$this->registerValue("yubopoints","_statsdb_yubototal");
	function _statsdb_yubototal($yubo,$_P,$_CB) {
		global $cdata,$DBc,$statsdb;
		$_IDENT = "yubopoints";

		#$DBc->sendSQL("UPDATE stat_players SET sp_yubototal='".$yubo."' WHERE sp_char='".$cdata['cid']."'","NONE");
		$statsdb->setValue('sp_yubototal',$yubo);

		$_P->unregisterValue($_IDENT,$_CB);
	}

	$this->registerValue("petcount","_statsdb_mekcount");
	function _statsdb_mekcount($count,$_P,$_CB) {
		global $cdata,$DBc,$statsdb;
		$_IDENT = "petcount";

		#$DBc->sendSQL("UPDATE stat_players SET sp_mekcount='".$count."' WHERE sp_char='".$cdata['cid']."'","NONE");
		$statsdb->setValue('sp_mekcount',$count);

		$_P->unregisterValue($_IDENT,$_CB);
	}

	$this->registerEntity("skilllist","_statsdb_maxlevel");
	function _statsdb_maxlevel($skills,$_P,$_CB) {
		global $cdata,$DBc,$log,$statsdb;
		$_IDENT = "skilllist";

		#$log->logf("rcv skilllist: ".var_export($skills,true));

		$lvl = 0;
		foreach($skills->skills as $elem) {
			if($elem->current > $lvl) {
				$lvl = $elem->current;
			}
		}

		#$DBc->sendSQL("UPDATE stat_players SET sp_maxlevel='".$lvl."' WHERE sp_char='".$cdata['cid']."'","NONE");
		$statsdb->setValue('sp_maxlevel',$lvl);

		$_P->unregisterEntity($_IDENT,$_CB);
	}

	$this->registerValue("_guildid","_statsdb_guildid");
	function _statsdb_guildid($id,$_P,$_CB) {
		global $cdata,$DBc,$statsdb;
		$_IDENT = "_guildid";

		#$DBc->sendSQL("UPDATE stat_players SET sp_guildid='".$id."' WHERE sp_char='".$cdata['cid']."'","NONE");
		$statsdb->setValue('sp_guildid',$id);

		$_P->unregisterValue($_IDENT,$_CB);
	}

	$this->registerValue("itemcount","_statsdb_itemcount");
	function _statsdb_itemcount($count,$_P,$_CB) {
		global $cdata,$DBc,$statsdb;
		$_IDENT = "itemcount";

		#$DBc->sendSQL("UPDATE stat_players SET sp_itemcount='".$count."' WHERE sp_char='".$cdata['cid']."'","NONE");
		$statsdb->setValue('sp_itemcount',$count);

		$_P->unregisterValue($_IDENT,$_CB);
	}
?>