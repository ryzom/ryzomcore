<?php
	class ValueCache extends DataSource {
		function ValueCache() {
			parent::__construct();
		}
		
		function getData($ident,$field,$type) {
			$res = $DBc->sendSQL("SELECT apv_value,apv_date FROM ach_player_valuecache WHERE apv_name='".$DBc->mre($field)."' AND apv_player='".$DBc->mre($ident)."'","ARRAY");

			return array($res[0]['apv_value'],$res[0]['apv_date']);
		}

		function writeData($ident,$field,$data,$type) {
			global $DBc;

			$DBc->sendSQL("INSERT INTO ach_player_valuecache (apv_name,apv_player,apv_value,apv_date) VALUES ('".$DBc->mre($field)."','".$DBc->mre($ident)."','".$DBc->mre($data)."','".time()."') ON DUPLICATE KEY UPDATE apv_value='".$DBc->mre($data)."', apv_date='".time()."'","NONE");
		}
	}
?>