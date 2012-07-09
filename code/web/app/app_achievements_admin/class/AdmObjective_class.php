<?php
	class AdmObjective extends AchObjective implements ADM {
		use AdmDispatcher;
		
		function AdmObjective($data,$parent) {
			parent::__construct($data,$parent);

			global $DBc;

			$res = $DBc->sqlQuery("SELECT atom_id FROM ach_atom WHERE atom_objective='".$this->getID()."'");
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$this->addChild($this->makeChild($res[$i]));
			}
		}

		protected function makeChild($d) {
			return new AdmAtom($d,$this);
		}

		function delete_me() {
			global $DBc;

			$DBc->sqlQuery("DELETE FROM ach_objective WHERE ao_id='".$this->getID()."'");
			$DBc->sqlQuery("DELETE FROM ach_player_objective WHERE apo_objective='".$this->getID()."'");

			$iter = $this->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				$curr->delete_me();
				$this->removeChild($curr->getID());
			}
		}

		function update() {
			global $DBc;

			$DBc->sqlQuery("UPDATE ach_objective SET ao_condition='".mysql_real_escape_string($this->getCondition())."',ao_value=".mkn($this->getValue()).",ao_display='".mysql_real_escape_string($this->getDisplay())."',ao_metalink=".mkn($this->getMetaImage())." WHERE ao_id='".$this->getID()."'");

			$DBc->sqlQuery("INSERT INTO ach_objective_lang (aol_objective,aol_lang,aol_name) VALUES ('".$this->getID()."','en','".mysql_real_escape_string($this->getName())."') ON DUPLICATE KEY UPDATE aol_name='".mysql_real_escape_string($this->getName())."'");
		}

		function insert() {
			global $DBc;

			$DBc->sqlQuery("INSERT INTO ach_objective (ao_perk,ao_condition,ao_value,ao_display,ao_metalink) VALUES ('".$this->getPerk()."','".mysql_real_escape_string($this->getCondition())."',".mkn($this->getValue()).",'".mysql_real_escape_string($this->getDisplay())."',".mkn($this->getMetaImage()).")");
			$id = mysql_insert_id();
			$this->setID($id);

			$DBc->sqlQuery("INSERT INTO ach_objective_lang (aol_objective,aol_lang,aol_name) VALUES ('".$this->getID()."','en','".mysql_real_escape_string($this->getName())."')");
		}

		function setCondition($c) {
			$this->condition = $c;
		}

		function setDisplay($d) {
			$this->display = $d;
		}

		function setName($n) {
			$this->name = $n;
		}

		function setValue($v) {
			$this->value = $v;
		}

		function setMetalink($m) {
			$this->meta_image = $m;
		}

		function setPerk($p) {
			$this->perk = $p;
		}
	}
?>