<?php
	class AdmObjective extends AchObjective implements ADM, AdmDispatcher {
		
		function AdmObjective($data,$parent) {
			parent::__construct($data,$parent);

			global $DBc;

			$res = $DBc->sqlQuery("SELECT atom_id FROM ach_atom WHERE atom_objective='".$this->getID()."'");
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$this->nodes[] = $this->makeChild($res[$i]);
			}
		}

		private function makeChild($d) {
			return new AdmAtom($d,$this);
		}

		function insertNode(&$n) { // insert an Atom
			$n->insert();
			$this->nodes[] = $n;
		}

		function removeNode($id) { // remove an Atom
			$res = $this->getNode($id);
			if($res != null) {
				$res->delete_me();
				$this->unsetChild($id);
			}
		}

		function updateNode($id,$data) { // update an Atom
			$res = $this->getNode($id);
			if($res != null) {
				#MISSING: set new data
				#
				$res->update();
			}
		}

		function getNode($id) { // find an atom
			foreach($this->nodes as $elem) {
				if($elem->getID == $id) {
					return $elem;
				}
			}

			return null;
		}

		function delete_me() {
			global $DBc;

			$DBc->sqlQuery("DELETE FROM ach_objective WHERE ao_id='".$this->getID()."'");
			$DBc->sqlQuery("DELETE FROM ach_player_objective WHERE apo_objective='".$this->getID()."'");

			foreach($this->nodes as $elem) {
				$elem->delete_me();
			}
		}

		function update() {

		}

		function insert() {

		}

		function setInDev($tf) {

		}
	}
?>