<?php
	class ValueCache extends DataSource {
		function ValueCache() {
			parent::__construct();
		}

		function loadData($ident,$type) {
			$res = $DBc->sendSQL("SELECT apv_value,apv_date,apv_name FROM ach_player_valuecache WHERE apv_player='".$DBc->mre($ident)."'","ARRAY");

			$this->data[$ident][$type] = new DataTable($res);
		}

		function writeData($ident,$type,$keys,$data) {
			global $DBc;

			$DBc->sendSQL("INSERT INTO ach_player_valuecache (apv_name,apv_player,apv_value,apv_date) VALUES ('".$DBc->mre($keys[0])."','".$DBc->mre($ident)."','".$DBc->mre($data[0])."','".time()."') ON DUPLICATE KEY UPDATE apv_value='".$DBc->mre($data)."', apv_date='".time()."'","NONE");
		}
	}
?>