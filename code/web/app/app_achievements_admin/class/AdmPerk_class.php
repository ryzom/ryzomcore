<?php
	class AdmPerk extends AchPerk implements ADM, AdmDispatcher {
		
		function AdmPerk($data,$parent) {
			parent::__construct($data,$parent);
		}

		protected function makeChild($d) {
			return new AdmObjective($d,$this);
		}

		function insertNode(&$n) { // insert an Objective
			$n->insert();
			$this->nodes[] = $n;
		}

		function removeNode($id) { // remove an Objective
			$res = $this->getNode($id);
			if($res != null) {
				$res->delete_me();
				$this->unsetChild($id);
			}
		}

		function updateNode($id,$data) { // update an Objective
			$res = $this->getNode($id);
			if($res != null) {
				#MISSING: set new data
				#
				$res->update();
			}
		}

		function getNode($id) { // find an Objective
			foreach($this->nodes as $elem) {
				if($elem->getID == $id) {
					return $tmp;
				}
			}

			return null;
		}

		function delete_me() {
			global $DBc;

			$DBc->sqlQuery("DELETE FROM ach_perk WHERE ap_id='".$this->getID()."'");
			$DBc->sqlQuery("DELETE FROM ach_player_perk WHERE app_perk='".$this->getID()."'");

			foreach($this->nodes as $elem) {
				$elem->delete_me();
				$this->unsetChild($elem->getID());
			}
		}

		function update() {

		}

		function insert() {

		}

		function unsetChild($id) { // remove child with given ID from nodes list; unset should destruct it.
			foreach($this->nodes as $key=>$elem) {
				if($elem->getID() == $id) {
					unset($this->nodes[$key]);
					return null;
				}
			}
		}

		function setInDev($tf) {

		}
	}
?>