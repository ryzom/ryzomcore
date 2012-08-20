<?php
	/*
	 * The ValueCache allows to store data that persists outside from actual atom evaluation. One might want to check
	 * if a value changes by xx since the last time parsing it, just as an example.
	 */
	class ValueCache {
		private $char;

		function ValueCache() {
			$this->char = false;
		}

		function setChar($c) { // select the character
			$this->char = $c;
		}

		function writeData($key,$val) { // write to cache
			global $DBc;

			$DBc->sendSQL("INSERT INTO ach_player_valuecache (apv_name,apv_player,apv_value,apv_date) VALUES ('".$this->user."','".$DBc->mre($key)."','".$DBc->mre($val)."','".time()."') ON DUPLICATE KEY UPDATE apv_value='".$DBc->mre($val)."', apv_date='".time()."'","NONE");
		}

		function getData($key) { // read from cache
			global $DBc;

			$res = $DBc->sendSQL("SELECT apv_value as value, apv_date as date FROM ach_player_valuecache WHERE apv_name='".$DBc->mre($key)."' AND apv_player='".$this->char."'","ARRAY");

			return $res[0];
		}
	}
?>