<?php
	/*
	 * Class for Atoms.
	 * It's used to run the code from the ruleset and register listening to data.
	 * Also we have the functions to manipulate progress here.
 	 */

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

		function register() { // register the atom's ruleset code

			try {
				return eval($this->ruleset);
			}
			catch(Exception $e) {
				echo $e->getMessage();
			}

			return null;
		}

		function registerValue($name,$func) { // register to listen for a value
			global $_DISPATCHER;
			
			$tmp = new Callback($this,$func);
			$_DISPATCHER->registerValue($name,$tmp);
		}

		function unregisterValue($name,$callback) { // unregister listening
			global $_DISPATCHER;

			$_DISPATCHER->unregisterValue($name,$callback);
		}

		function registerEntity($name,$func) { // register to listen for an entity
			global $_DISPATCHER;
			
			$tmp = new Callback($this,$func);
			$_DISPATCHER->registerEntity($name,$tmp);
		}

		function unregisterEntity($name,$callback) { // unregister
			global $_DISPATCHER;

			$_DISPATCHER->unregisterEntity($name,$callback);
		}

		function grant($count = 1) { // grant an atom
			global $DBc,$atom_insert;

			#$DBc->sendSQL("INSERT INTO ach_player_atom (apa_atom,apa_player,apa_date,apa_expire,apa_state,apa_value) VALUES ('".$this->id."','".$this->user['cid']."','".time()."',null,'GRANT','".$count."')","NONE");

			$atom_insert[] = "('".$this->id."','".$this->user['cid']."','".time()."',null,'GRANT','".$count."')";
		}

		function deny() { // deny an atom
			global $DBc;

			$DBc->sendSQL("INSERT INTO ach_player_atom (apa_atom,apa_player,apa_date,apa_expire,apa_state) VALUES ('".$this->id."','".$this->user['cid']."','".time()."',null,'DENY','1')","NONE");
		}

		function reset_() { // reset progress for this atom
			global $DBc;

			#$res = $DBc->sendSQL("SELECT * FROM ach_player_atom WHERE apa_atom='".$this->id."' AND apa_player='".$this->user['cid']."'","ARRAY");

			$DBc->sendSQL("DELETE FROM ach_player_atom WHERE apa_atom='".$this->id."' AND apa_player='".$this->user['cid']."'","NONE");
		}

		function reset_all() { // reset progress for all atoms of the same objective
			global $DBc;

			$res = $DBc->sendSQL("SELECT atom_id FROM ach_atom WHERE atom_objective='".$this->objective."'","ARRAY");
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$DBc->sendSQL("DELETE FROM ach_player_atom WHERE apa_atom='".$res[$i]['atom_id']."' AND apa_player='".$this->user['cid']."'","NONE");
			}
		}

		function unlock() { // unlock atom
			global $DBc;

			$DBc->sendSQL("DELETE FROM ach_player_atom WHERE apa_atom='".$this->id."' AND apa_player='".$this->user['cid']."' AND apa_state='DENY'","NONE");
		}

		function unlock_all() { // unlock all atoms of the same objective
			global $DBc;

			$res = $DBc->sendSQL("SELECT atom_id FROM ach_atom WHERE atom_objective='".$this->objective."'","ARRAY");
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$DBc->sendSQL("DELETE FROM ach_player_atom WHERE apa_atom='".$res[$i]['atom_id']."' AND apa_player='".$this->user['cid']."' AND apa_state='DENY'","NONE");
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