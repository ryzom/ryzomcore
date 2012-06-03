<?php
	class ValueCache extends DataSource {
		function ValueCache() {
			$this->types[] = "c_cache";

			$this->write = true;
		}
		
		function getData($type,$ident,$field) {

		}

		function writeData($type,$ident,$field = array(),$value = array()) {
			global $DBc;

			if($type == "c_cache") {
				$DBc->sendSQL("INSERT INTO ach_player_valuecache () VALUES () ON DUPLICATE KEY UPDATE ");

				return true;
			}
			else {
				return false;
			}
		}
	}
?>