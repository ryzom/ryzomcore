<?php
	class AchTask extends Parentum implements Tieable {
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
		protected $inherit_obj;
		private $heritage_list;
		protected $tie_race;
		#protected $tie_cult;
		#protected $tie_civ;
		protected $tie_align;

		function AchTask($data,$parent) {
			global $DBc,$_USER,$_CONF;

			parent::__construct();

			#$this->heritage_list = array();
			
			$this->setParent($parent);
			$this->setID($data['at_id']);
			$this->achievement = $data['at_achievement'];
			$this->value = $data['at_value'];
			$this->name = $data['atl_name'];
			$this->done = $data['apt_date'];
			$this->dev = $data['at_dev'];
			$this->template = $data['atl_template'];
			$this->parent_id = $data['at_parent'];
			$this->inherit_obj = $data['at_inherit'];

			if($this->inherit_obj == 1) {
				$this->heritage_list = new AVLTree();
			}
			else {
				$this->heritage_list = null;
			}

			if($this->name == null) {
				$res = $DBc->sqlQuery("SELECT * FROM ach_task_lang WHERE atl_lang='".$_CONF['default_lang']."' AND atl_task='".$this->id."'");
				$this->name = $res[0]['atl_name'];
				$this->template = $res[0]['atl_template'];
			}

			$res = $DBc->sqlQuery("SELECT * FROM ach_objective LEFT JOIN (ach_objective_lang) ON (aol_lang='".$_USER->getLang()."' AND aol_objective=ao_id) LEFT JOIN (ach_player_objective) ON (apo_objective=ao_id AND apo_player='".$_USER->getID()."') LEFT JOIN (ach_achievement,ach_achievement_lang) ON (aa_id=ao_metalink AND aa_id=aal_achievement AND aal_lang='".$_USER->getLang()."') WHERE ao_task='".$this->id."' ORDER by aol_name ASC,aal_name ASC");

			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$this->addChild($this->makeChild($res[$i]));
			}

			//load ties
			$res = $DBc->sqlQuery("SELECT attr_race FROM ach_task_tie_race WHERE attr_task='".$this->id."'");
			$sz = sizeof($res);

			$this->tie_race = array();
			for($i=0;$i<$sz;$i++) {
				$this->tie_race[] = $res[$i]['attr_race'];
			}

			/*$res = $DBc->sqlQuery("SELECT attcult_cult FROM ach_task_tie_cult WHERE attcult_task='".$this->id."'");
			$sz = sizeof($res);

			$this->tie_cult = array();
			for($i=0;$i<$sz;$i++) {
				$this->tie_cult[] = $res[$i]['attcult_cult'];
			}

			$res = $DBc->sqlQuery("SELECT attciv_civ FROM ach_task_tie_civ WHERE attciv_task='".$this->id."'");
			$sz = sizeof($res);

			$this->tie_civ = array();
			for($i=0;$i<$sz;$i++) {
				$this->tie_civ[] = $res[$i]['attciv_civ'];
			}*/

			$res = $DBc->sqlQuery("SELECT atta_alignment FROM ach_task_tie_align WHERE atta_task='".$this->id."'");
			$sz = sizeof($res);

			$this->tie_align = array();
			for($i=0;$i<$sz;$i++) {
				$this->tie_align[] = $res[$i]['atta_alignment'];
			}
		}

		function loadHeritage() {
			if($this->inherit_obj == 0) {
				return false;
			}
			$child = $this->parent->getChildDataByID($this->parent_id);
			if($child == null) {
				return false;
			}
			$iter = $child->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				$this->addChild($curr);
				$this->heritage_list->insert($curr);
			}
			return true;
		}
		
		#@override Parentum::makeChild()
		protected function makeChild($a) {
			return new AchObjective($a,$this);
		}

		function getHeritage() {
			return $this->inherit_obj;
		}

		function isInherited($id) {
			if($this->getHeritage() == 0) {
				return false;
			}
			if($this->heritage_list == null) {
				return false;
			}
			return ($this->heritage_list->find($id) != null);
		}

		function hasTieRace() {
			if($this->dev == 1) {
				return false;
			}
			return (sizeof($this->tie_race) != 0);
		}

		function hasTieAlign() {
			if($this->dev == 1) {
				return false;
			}
			return (sizeof($this->tie_align) != 0);
		}

		function hasTieRace_open() {
			return $this->hasTieRace();
		}

		function hasTieRace_done() {
			return $this->hasTieRace();
		}

		function hasTieAlign_open() {
			return $this->hasTieAlign();
		}

		function hasTieAlign_done() {
			return $this->hasTieAlign();
		}

		function hasTieRaceDev() {
			return (sizeof($this->tie_race) != 0);
		}

		function hasTieAlignDev() {
			return (sizeof($this->tie_align) != 0);
		}

		function isTiedRace($r) {
			if(sizeof($this->tie_race) == 0) {
				return true;
			}
			return in_array($r,$this->tie_race);
		}

		function isTiedAlign($cult,$civ) {
			if($cult == "%" || $civ == "%") {
				return true;
			}
			if(sizeof($this->tie_align) == 0) {
				return true;
			}
			return in_array(($cult.'|'.$civ),$this->tie_align);
		}

		function isTiedRace_open($r) {
			return $this->isTiedRace($r);
		}

		function isTiedRace_done($r) {
			return $this->isTiedRace($r);
		}

		function isTiedAlign_done($cult,$civ) {
			return $this->isTiedAlign($cult,$civ);
		}

		function isTiedAlign_open($cult,$civ) {
			return $this->isTiedAlign($cult,$civ);
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