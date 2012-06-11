<?php
	class Atom {
		private $ruleset;
		private $id;
		private $objective;

		function Atom(&$data) {
			$this->ruleset = $data['atom_ruleset_parsed'];

			$this->id = $data['atom_id'];
			$this->objective = $data['atom_objective'];
		}

		function evalRuleset($user) {
			global $DBc,$_DATA;

			try {
				return eval($this->ruleset);
			}
			catch(Exception $e) {
				return $e->getMessage()
			}
		}

		private function grant($user,$condition) {
			global $DBc;

			$DBc->sendSQL("INSERT INTO ach_player_atom (apa_atom,apa_player,apa_date,apa_expire,apa_state) VALUES ('".$this->id."','".$user."','".date()."','".$DBc->mre($condition)."','GRANT')","NONE");
		}

		private function deny($user,$condition) {
			global $DBc;

			$DBc->sendSQL("INSERT INTO ach_player_atom (apa_atom,apa_player,apa_date,apa_expire,apa_state) VALUES ('".$this->id."','".$user."','".date()."','".$DBc->mre($condition)."','DENY')","NONE");
		}

		private function reset_($user) {
			global $DBc;

			$DBc->sendSQL("DELETE FROM ach_player_atom WHERE apa_atom='".$this->id."' AND apa_player='".$user."'","NONE");
		}

		private function reset_all($user) {
			global $DBc;

			$res = $DBc->sendSQL("SELECT atom_id FROM ach_atom WHERE atom_objective='".$this->objective."'","ARRAY");
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$DBc->sendSQL("DELETE FROM ach_player_atom WHERE apa_atom='".$res[$i]['atom_id']."' AND apa_player='".$user."'","NONE");
			}
		}

		private function unlock($user) {
			global $DBc;

			$DBc->sendSQL("DELETE FROM ach_player_atom WHERE apa_atom='".$this->id."' AND apa_player='".$user."' AND apa_state='DENY'","NONE");
		}

		private function unlock_all($user) {
			global $DBc;

			$res = $DBc->sendSQL("SELECT atom_id FROM ach_atom WHERE atom_objective='".$this->objective."'","ARRAY");
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$DBc->sendSQL("DELETE FROM ach_player_atom WHERE apa_atom='".$res[$i]['atom_id']."' AND apa_player='".$user."' AND apa_state='DENY'","NONE");
			}
		}

		function getID() {
			return $this->id;
		}

		function getObjective() {
			return $this->objective;
		}
	}
?>