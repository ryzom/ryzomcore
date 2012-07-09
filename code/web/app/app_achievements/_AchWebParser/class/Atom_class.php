<?php
	class Atom {
		private $ruleset;
		private $id;
		private $objective;
		private $user;

		function Atom(&$data,$user) {
			$this->ruleset = $data['atom_ruleset_parsed'];

			$this->id = $data['atom_id'];
			$this->objective = $data['atom_objective'];

			$this->user = $user;
		}

		function register() {
			global $DBc,$_DATA;

			echo "register<br>";

			try {
				return eval($this->ruleset);
			}
			catch(Exception $e) {
				echo $e->getMessage();
			}
		}

		function registerValue($name,$func) {
			global $_DISPATCHER;
			
			$tmp = new Callback($this,$func);
			$_DISPATCHER->registerValue($name,$tmp);
		}

		function unregisterValue($name,$callback) {
			global $_DISPATCHER;

			$_DISPATCHER->unregisterValue($name,$callback);
		}

		function grant() {
			global $DBc;

			echo "G<br>";

			$DBc->sendSQL("INSERT INTO ach_player_atom (apa_atom,apa_player,apa_date,apa_expire,apa_state) VALUES ('".$this->id."','".$this->user."','".time()."',null,'GRANT')","NONE");
		}

		function deny() {
			global $DBc;

			$DBc->sendSQL("INSERT INTO ach_player_atom (apa_atom,apa_player,apa_date,apa_expire,apa_state) VALUES ('".$this->id."','".$this->user."','".time()."',null,'DENY')","NONE");
		}

		function reset_() {
			global $DBc;

			$DBc->sendSQL("DELETE FROM ach_player_atom WHERE apa_atom='".$this->id."' AND apa_player='".$this->user."'","NONE");
		}

		function reset_all() {
			global $DBc;

			$res = $DBc->sendSQL("SELECT atom_id FROM ach_atom WHERE atom_objective='".$this->objective."'","ARRAY");
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$DBc->sendSQL("DELETE FROM ach_player_atom WHERE apa_atom='".$res[$i]['atom_id']."' AND apa_player='".$this->user."'","NONE");
			}
		}

		function unlock() {
			global $DBc;

			$DBc->sendSQL("DELETE FROM ach_player_atom WHERE apa_atom='".$this->id."' AND apa_player='".$this->user."' AND apa_state='DENY'","NONE");
		}

		function unlock_all() {
			global $DBc;

			$res = $DBc->sendSQL("SELECT atom_id FROM ach_atom WHERE atom_objective='".$this->objective."'","ARRAY");
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$DBc->sendSQL("DELETE FROM ach_player_atom WHERE apa_atom='".$res[$i]['atom_id']."' AND apa_player='".$this->user."' AND apa_state='DENY'","NONE");
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