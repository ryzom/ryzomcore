<?php
	class AdmTask extends AchTask implements ADM {
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

		protected $condition;
		protected $condition_value;
		protected $torder;

		function AdmTask($data,$parent) {
			parent::__construct($data,$parent);

			$this->condition = $data["at_condition"];
			$this->condition_value = $data["at_condition_value"];
			$this->torder = $data["at_torder"];
		}

		protected function makeChild($d) {
			return new AdmObjective($d,$this);
		}

		function getLang($lang) { // load language
			global $DBc;

			$res = $DBc->sqlQuery("SELECT * FROM ach_task_lang WHERE atl_task='".$this->getID()."' AND atl_lang='".$lang."'");

			return array(0=>$res[0]['atl_name'],1=>$res[0]['atl_template']);
		}

		function setLang($lang,$txt,$tpl) { // write language
			global $DBc,$_USER;

			$DBc->sqlQuery("INSERT INTO ach_task_lang (atl_task,atl_lang,atl_name,atl_template) VALUES ('".$this->getID()."','".$DBc->sqlEscape($lang)."','".$DBc->sqlEscape($txt)."',".mkn($tpl).") ON DUPLICATE KEY UPDATE atl_name='".$DBc->sqlEscape($txt)."',atl_template=".mkn($tpl)."");

			if($_USER->getLang() == $lang) {
				$this->name = $txt;
				$this->template = $tpl;
			}
		}

		function delete_me() {
			global $DBc;

			$DBc->sqlQuery("DELETE FROM ach_task WHERE at_id='".$this->getID()."'");
			$DBc->sqlQuery("DELETE FROM ach_player_task WHERE apt_task='".$this->getID()."'");
			$DBc->sqlQuery("DELETE FROM ach_task_tie_align WHERE atta_task='".$this->getID()."'");
			$DBc->sqlQuery("DELETE FROM ach_task_tie_race WHERE attr_task='".$this->getID()."'");

			$iter = $this->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				$curr->delete_me();
				$this->removeChild($curr->getID());
			}
		}

		function update() {
			global $DBc;

			$DBc->sqlQuery("UPDATE ach_task SET at_parent=".mkn($this->getParentID()).",at_value='".$DBc->sqlEscape($this->getValue())."',at_condition='".$DBc->sqlEscape($this->getCondition())."',at_condition_value=".mkn($this->getConditionValue()).",at_dev='".$this->getDev()."',at_torder='".$this->torder."', at_inherit='".$this->inherit_obj."' WHERE at_id='".$this->getID()."'");

			$DBc->sqlQuery("INSERT INTO ach_task_lang (atl_task,atl_lang,atl_name,atl_template) VALUES ('".$this->getID()."','en','".$DBc->sqlEscape($this->getName())."',".mkn($this->getTemplate()).") ON DUPLICATE KEY UPDATE atl_name='".$DBc->sqlEscape($this->getName())."',atl_template=".mkn($this->getTemplate())."");

			$DBc->sqlQuery("DELETE FROM ach_task_tie_align WHERE atta_task='".$this->getID()."'");
			$DBc->sqlQuery("DELETE FROM ach_task_tie_race WHERE attr_task='".$this->getID()."'");

			foreach($this->tie_race as $elem) {
				$DBc->sqlQuery("INSERT INTO ach_task_tie_race (attr_task,attr_race) VALUES ('".$this->getID()."','".$DBc->sqlEscape($elem)."')");
			}

			foreach($this->tie_align as $elem) {
				$DBc->sqlQuery("INSERT INTO ach_task_tie_align (atta_task,atta_alignment) VALUES ('".$this->getID()."','".$DBc->sqlEscape($elem)."')");
			}
		}

		function insert() {
			global $DBc;

			$this->dev = 1;

			$DBc->sqlQuery("INSERT INTO ach_task (at_achievement,at_parent,at_value,at_condition,at_condition_value,at_dev,at_torder,at_inherit) VALUES ('".$this->getAchievement()."',".mkn($this->getParentID()).",'".$DBc->sqlEscape($this->getValue())."','".$DBc->sqlEscape($this->getCondition())."',".mkn($this->getConditionValue()).",'1','".$this->torder."','".$this->inherit_obj."')");
			$id = $DBc->insertID();
			$this->setID($id);

			$DBc->sqlQuery("INSERT INTO ach_task_lang (atl_task,atl_lang,atl_name,atl_template) VALUES ('".$this->getID()."','en','".$DBc->sqlEscape($this->getName())."',".mkn($this->getTemplate()).")");

			foreach($this->tie_race as $elem) {
				$DBc->sqlQuery("INSERT INTO ach_task_tie_race (attr_task,attr_race) VALUES ('".$this->getID()."','".$DBc->sqlEscape($elem)."')");
			}

			foreach($this->tie_align as $elem) {
				$DBc->sqlQuery("INSERT INTO ach_task_tie_align (atta_task,atta_alignment) VALUES ('".$this->getID()."','".$DBc->sqlEscape($elem)."')");
			}
		}

		function setAchievement($a) {
			$this->achievement = $a;
		}

		function setName($name) {
			$this->name = $name;
		}

		function setTemplate($t) {
			$this->template = $t;
		}

		function setValue($v) {
			$this->value = $v;
		}

		function setTieRace($t) {
			$this->tie_race = $t;
		}

		function setTieAlign($t) {
			$this->tie_align = $t;
		}

		function getCondition() {
			return $this->condition;
		}

		function getConditionValue() {
			return $this->condition_value;
		}

		function setCondition($c) {
			$this->condition = $c;
		}

		function setConditionValue($v) {
			$this->condition_value = $v;
		}

		function getTorder() {
			return $this->torder;
		}

		function setTorder($t) {
			$this->torder = $t;
		}

		function setHeritage($i) {
			$this->inherit_obj = $i;
		}

		function setParentID($p,$order = true) { #reordering must happen A) after insert B) when updating
			if($p == null || $p == "null") {

				$this->parent_id = null;

			}
			else {

				$this->parent_id = $p;

			}

			if($order == true) {

				$iter = $this->parent->getIterator();
				while($iter->hasNext()) {
					$curr = $iter->getNext();
					if($curr->getID() == $this->id) {
						continue;
					}
					if($curr->getParentID() == $this->parent_id) {
						$curr->setParentID($this->id,false);
						$curr->update();

						break;
					}
				}
			}
		}

		function isTiedRace($r) {
			if(sizeof($this->tie_race) == 0) {
				return false;
			}
			return in_array($r,$this->tie_race);
		}

		function isTiedAlign($cult,$civ) {
			if(sizeof($this->tie_align) == 0) {
				return false;
			}
			return in_array(($cult.'|'.$civ),$this->tie_align);
		}
	}
?>