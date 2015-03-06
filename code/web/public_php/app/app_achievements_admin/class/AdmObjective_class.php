<?php
	class AdmObjective extends AchObjective implements ADM {
		#########################
		# PHP 5.3 compatible
		# AdmDispatcher_trait replaces this in PHP 5.4

		function insertNode($n) {
			$n->setParent($this);
			$n->insert();
			$this->addChild($n);
		}

		function removeNode($id) {
			$res = $this->getChildDataByID($id);
			if($res != null) {
				$res->delete_me();
				$this->removeChild($id);
			}
		}

		function updateNode($id) { // PROBABLY USELESS!
			$res = $this->getChildDataByID($id);
			if($res != null) {
				$res->update();
			}
		}

		function getPathID($path = "") {
			if($path != "") {
				$path = ";".$path;
			}
			$path = $this->getID().$path;
			if($this->parent != null) {
				return $this->parent->getPathID($path);
			}

			return $path;
		}

		function getElementByPath($pid) {
			$tmp = explode(";",$pid);
			if($tmp[0] == $this->getID()) {
				if(sizeof($tmp) > 1) {
					$c = $this->getChildDataByID($tmp[1]);
					if($c != null) {
						unset($tmp[0]);
						return $c->getElementByPath(implode(";",$tmp));
					}
					return null;
				}
				else {
					return $this;
				}
			}
			return null;
		}
		#########################
		
		function AdmObjective($data,$parent) {
			parent::__construct($data,$parent);

			global $DBc;

			$res = $DBc->sqlQuery("SELECT * FROM ach_atom WHERE atom_objective='".$this->getID()."'");
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$this->addChild($this->makeChild($res[$i]));
			}
		}

		protected function makeChild($d) {
			return new AdmAtom($d,$this);
		}

		function getLang($lang) { // load language
			global $DBc;

			$res = $DBc->sqlQuery("SELECT * FROM ach_objective_lang WHERE aol_objective='".$this->getID()."' AND aol_lang='".$lang."'");

			return $res[0]['aol_name'];
		}

		function setLang($lang,$txt) { // write language
			global $DBc,$_USER;

			$DBc->sqlQuery("INSERT INTO ach_objective_lang (aol_objective,aol_lang,aol_name) VALUES ('".$this->getID()."','".$DBc->sqlEscape($lang)."','".$DBc->sqlEscape($txt)."') ON DUPLICATE KEY UPDATE aol_name='".$DBc->sqlEscape($txt)."'");

			if($_USER->getLang() == $lang) {
				$this->name = $txt;
			}
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

			$DBc->sqlQuery("UPDATE ach_objective SET ao_condition='".$DBc->sqlEscape($this->getCondition())."',ao_value=".mkn($this->getValue()).",ao_display='".$DBc->sqlEscape($this->getDisplay())."',ao_metalink=".mkn($this->getMetalink())." WHERE ao_id='".$this->getID()."'");

			$DBc->sqlQuery("INSERT INTO ach_objective_lang (aol_objective,aol_lang,aol_name) VALUES ('".$this->getID()."','en','".$DBc->sqlEscape($this->getName())."') ON DUPLICATE KEY UPDATE aol_name='".$DBc->sqlEscape($this->getName())."'");
		}

		function insert() {
			global $DBc;

			$DBc->sqlQuery("INSERT INTO ach_objective (ao_task,ao_condition,ao_value,ao_display,ao_metalink) VALUES ('".$this->getTask()."','".$DBc->sqlEscape($this->getCondition())."',".mkn($this->getValue()).",'".$DBc->sqlEscape($this->getDisplay())."',".mkn($this->getMetalink()).")");
			$id = $DBc->insertID();
			$this->setID($id);

			$DBc->sqlQuery("INSERT INTO ach_objective_lang (aol_objective,aol_lang,aol_name) VALUES ('".$this->getID()."','en','".$DBc->sqlEscape($this->getName())."')");
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
			$this->metalink = $m;
			if($this->getDisplay() == "meta") {
				$this->name = "<i>name and image will load on refresh only!</i>";
			}
		}

		function setTask($t) {
			$this->task = $t;
		}
	}
?>