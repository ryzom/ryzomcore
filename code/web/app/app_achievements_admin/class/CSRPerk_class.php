<?php
	class CSRPerk extends AchPerk implements CSR {
		
		function CSRPerk(&$data) {
			parent::__construct($data);
		}

		function grant($pid) {
			global $DBc;

			$DBc->sqlQuery("INSERT INTO ach_player_perk (app_perk,app_player,app_date) VALUES ('".$this->getID()."','".$pid."','".time()."')");

			foreach($this->nodes as $elem) {
				$elem->grant();
			}
		}

		function deny($pid) {
			global $DBc;
			
			$DBc->sqlQuery("DELETE FROM ach_player_perk WHERE app_perk='".$this->getID()."' AND app_player='".$pid."'");

			foreach($this->nodes as $elem) {
				$elem->deny($pid);
			}
		}
	}
?>