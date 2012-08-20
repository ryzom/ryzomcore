<?php
	class AchTask extends Parentum {
		#########################
		# PHP 5.3 compatible
		# InDev_trait replaces this in PHP 5.4
		protected $dev;

		function inDev() {
			return ($this->dev == 1);
		}

		function getDev() {
			return $this->dev;
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
		#########################

		protected $achievement;
		protected $value;
		protected $name;
		protected $done;
		protected $template;
		protected $parent_id;

		function AchTask($data,$parent) {
			global $DBc,$_USER;

			parent::__construct();
			
			$this->setParent($parent);
			$this->setID($data['at_id']);
			$this->achievement = $data['at_achievement'];
			$this->value = $data['at_value'];
			$this->name = $data['atl_name'];
			$this->done = $data['apt_date'];
			$this->dev = $data['at_dev'];
			$this->template = $data['atl_template'];
			$this->parent_id = $data['at_parent'];

			$res = $DBc->sqlQuery("SELECT * FROM ach_objective LEFT JOIN (ach_objective_lang) ON (aol_lang='".$_USER->getLang()."' AND aol_objective=ao_id) LEFT JOIN (ach_player_objective) ON (apo_objective=ao_id AND apo_player='".$_USER->getID()."') LEFT JOIN (ach_achievement,ach_achievement_lang) ON (aa_id=ao_metalink AND aa_id=aal_achievement AND aal_lang='".$_USER->getLang()."') WHERE ao_task='".$this->id."' ORDER by aol_name ASC,aal_name ASC");
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$this->addChild($this->makeChild($res[$i]));
			}
		}
		
		#@override Parentum::makeChild()
		protected function makeChild($a) {
			return new AchObjective($a,$this);
		}

		function getAchievement() {
			return $this->achievement;
		}

		function getValue() {
			return $this->value;
		}

		function getDisplayName() {
			if(substr($this->name,0,1) == "!") {
				return substr($this->name,1);
			}
			return $this->parent->fillTemplate(explode(";",$this->name));
		}

		function getName() {
			return $this->name;
		}

		function objDrawable() {
			$iter = $this->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				if($curr->getDisplay() != "hidden") {
					return true;
				}
			}

			return false;
		}

		function isDone() {
			return ($this->done > 0);
		}

		function getDone() {
			return $this->done;
		}

		function fillTemplate($insert = array()) {
			if($this->template == null) {
				return implode(";",$insert);
			}
			else {
				$tmp = $this->template;
				$match = array();
				preg_match_all('#\[([0-9]+)\]#', $this->template, $match);
				foreach($match[0] as $key=>$elem) {
					$tmp = str_replace("[".$match[1][$key]."]",$insert[$key],$tmp);
				}
				return $tmp;
			}
		}
		
		function getTemplate() {
			return $this->template;
		}

		function getParentID() {
			return $this->parent_id;
		}
	}
?>