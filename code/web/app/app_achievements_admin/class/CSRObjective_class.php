<?php
	class CSRObjective extends AchObjective implements CSR {

		private $nodes;
		
		function CSRObjective(&$data) {
			parent::__construct($data);

			global $DBc;

			$res = $DBc->sqlQuery("SELECT atom_id FROM ach_atom WHERE atom_objective='".$this->getID()."'");
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$this->nodes[] = new CSRAtom($res[$i]);
			}
		}

		function grant($pid) {
			global $DBc;

			$DBc->sqlQuery("INSERT INTO ach_player_objective (apo_objective,apo_player,apo_date) VALUES ('".$this->getID()."','".$pid."','".time()."')");

			foreach($this->nodes as $elem) {
				$elem->grant($pid);
			}
		}

		function deny($pid) {
			global $DBc;

			$DBc->sqlQuery("DELETE FROM ach_player_objective WHERE apo_objective='".$this->getID()."' AND apo_player='".$pid."'");

			foreach($this->nodes as $elem) {
				$elem->deny($pid);
			}
		}
	}
?>