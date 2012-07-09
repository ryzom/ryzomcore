<?php
	class AchAchievement extends AchList {
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

		protected $parent_id;
		protected $category;
		protected $tie_race;
		protected $tie_civ;
		protected $tie_cult;
		protected $image;
		protected $name;
		protected $template;

		function AchAchievement($data,$parent) {
			global $DBc,$_USER;

			parent::__construct();
			
			$this->setParent($parent);
			$this->setID($data['aa_id']);
			$this->parent_id = $data['aa_parent'];
			$this->category = $data['aa_category'];
			$this->tie_race = $data['aa_tie_race'];
			$this->tie_civ = $data['aa_tie_civ'];
			$this->tie_cult = $data['aa_tie_cult'];
			$this->image = $data['aa_image'];
			$this->name = $data['aal_name'];
			$this->template = $data['aal_template'];
			$this->dev = $data['aa_dev'];

			$res = $DBc->sqlQuery("SELECT * FROM ach_perk LEFT JOIN (ach_perk_lang) ON (apl_lang='".$_USER->getLang()."' AND apl_perk=ap_id) LEFT JOIN (ach_player_perk) ON (app_perk=ap_id AND app_player='".$_USER->getID()."') WHERE ap_achievement='".$this->id."' ORDER by ap_porder ASC");
			
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$tmp = $this->makeChild($res[$i]);

				if($tmp->isDone()) {
					$this->addDone($tmp);
				}
				else {
					$this->addOpen($tmp);
				}
			}
		}

		protected function makeChild($a) {
			return new AchPerk($a,$this);
		}

		function unlockedByParent() {
			if($this->parent_id != null) {
				$tmp = $this->parent->getChildByID($this->parent_id);
				return ($tmp->hasOpen() == false);
			}

			return true;
		}

		function getParentID() {
			return $this->parent_id;
		}

		function getTieRace() {
			return $this->tie_race;
		}

		function getTieCiv() {
			return $this->tie_civ;
		}

		function getTieCult() {
			return $this->tie_cult;
		}

		function getImage() {
			return $this->image;
		}

		function getName() {
			return $this->name;
		}

		function getValueDone() {
			$val = 0;
			$iter = $this->getDone();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				$val += $curr->getValue();
			}
			return $val;
		}

		function getValueOpen() {
			$iter = $this->getOpen();
			if($iter->hasNext()) {
				$curr = $iter->getNext();
				return $curr->getValue();
			}
			return 0;
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

		function getCategory() {
			return $this->category;
		}
		
	}
?>