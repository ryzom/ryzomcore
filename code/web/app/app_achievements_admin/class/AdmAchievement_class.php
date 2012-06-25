<?php
	class AdmAchievement extends AchAchievement implements ADM, AdmDispatcher {
		
		function AdmAchievement($data,$parent) {
			parent::__construct($data,$parent);
		}

		protected function makeChild(&$d) {
			return new AdmPerk($d,$this);
		}

		function insertNode(&$n) { // add a Perk
			$n->insert();
			$this->nodes[] = $n;
		}

		function removeNode($id) { // remove a Perk
			$res = $this->getNode($id);
			if($res != null) {
				$res->delete_me();
				$this->unsetChild($id);
			}
		}

		function updateNode($id,$data) { // update a Perk
			$res = $this->getNode($id);
			if($res != null) {
				#MISSING: set new data
				#
				$res->update();
			}
		}

		function getNode($id) { // find a Perk
			foreach($this->nodes as $elem) {
				if($elem->getID == $id) {
					return $elem;
				}
			}

			return null;
		}

		function delete_me() {
			global $DBc;

			$DBc->sqlQuery("DELETE FROM ach_achievement WHERE aa_id='".$this->getID()."'");
			$DBc->sqlQuery("DELETE FROM ach_player_achievement WHERE apa_id='".$this->getID()."'");
			$DBc->sqlQuery("DELETE FROM ach_achievement_lang WHERE NOT EXISTS (SELECT * FROM ach_achievement WHERE aa_id=aal_achievement)");

			foreach($this->nodes as $elem) {
				$elem->delete_me();
				$this->unsetChild($elem->getID());
			}
		}

		function update() {
			global $DBc;

			$DBc->sqlQuery("UPDATE ach_achievement SET aa_parent='".$this->getParent())."',aa_tie_race='".mysql_real_escape_string($this->getTieRace())."',aa_tie_cult='".mysql_real_escape_string($this->getTieCult())."',aa_tie_civ='".mysql_real_escape_string($this->getTieCiv())."',aa_image='".mysql_real_escape_string($this->getImage())."',aa_dev='".$this->getDev()."' WHERE aa_id='".$this->geID()."'");

			#MISSING: update lang entry
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
			if($tf == true) {
				$this->setDev(1);
			}
			else {
				$this->setDev(0);
			}

			$this->update();
		}

		function setDev($d) {
			$this->dev = $d;
		}

		function setID($id) {
			$this->id = $id;
		}

		function setParent($p) {
			$this->parent = $p
		}

		function setCategory($c) {
			$this->category = $c;
		}

		function setTieRace($t) {
			$this->tie_race = $t;
		}

		function setTieCiv($t) {
			$this->tie_civ = $t;
		}

		function setTieCult($t) {
			$this->tie_cult = $t;
		}

		function setImage($i) {
			$this->image = $i;
		}

		function setName($n) {
			$this->name = $n;
		}

		function setTemplate($i) {
			$this->template = $t;
		}
	}
?>