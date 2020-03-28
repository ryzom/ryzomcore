<?php
	/*
	 * The Achievement class that holds one achievement. It is able to load one an the same task an treat is as both,
	 * open and done.
	 */

	class AchAchievement extends AchList implements Tieable {
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

		protected $parent_id;
		protected $category;
		#protected $tie_race;
		#protected $tie_civ;
		#protected $tie_cult;
		protected $image;
		protected $name;
		protected $template;
		protected $sticky;

		function AchAchievement($data,&$parent) {
			global $DBc,$_USER,$_CONF;

			parent::__construct();
			
			$this->setParent($parent); // real parent node
			$this->setID($data['aa_id']);
			$this->parent_id = $data['aa_parent']; // id of parent
			$this->category = $data['aa_category'];
			#$this->tie_race = $data['aa_tie_race'];
			#$this->tie_civ = $data['aa_tie_civ'];
			#$this->tie_cult = $data['aa_tie_cult'];
			$this->image = $data['aa_image'];
			$this->name = $data['aal_name'];
			$this->template = $data['aal_template'];
			$this->dev = $data['aa_dev'];
			$this->sticky = $data['aa_sticky'];

			if($this->name == null) {
				$res = $DBc->sqlQuery("SELECT * FROM ach_achievement_lang WHERE aal_lang='".$_CONF['default_lang']."' AND aal_achievement='".$this->id."'");
				$this->name = $res[0]['aal_name'];
				$this->template = $res[0]['aal_template'];
			}

			$res = $DBc->sqlQuery("SELECT * FROM ach_task LEFT JOIN (ach_task_lang) ON (atl_lang='".$_USER->getLang()."' AND atl_task=at_id) LEFT JOIN (ach_player_task) ON (apt_task=at_id AND apt_player='".$_USER->getID()."') WHERE at_achievement='".$this->id."' AND (NOT EXISTS (SELECT * FROM ach_task_tie_align WHERE atta_task=at_id) OR EXISTS (SELECT * FROM ach_task_tie_align WHERE atta_task=at_id AND atta_alignment LIKE '".$parent->getCurrentCult().'|'.$parent->getCurrentCiv()."')) ORDER by at_torder ASC");
			
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

			$iter = $this->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				$curr->loadHeritage();
			}
		}

		function parentDone() { // check if the parent is complete
			if($this->parent_id == null) {
				return true;
			}
			else {
				$p = $this->parent->getChildDataByID($this->parent_id);
				if($p == null) {
					return true;
				}

				return ($p->hasOpen() == false);
			}
		}

		#@override Parentum::makeChild()
		protected function makeChild($a) {
			return new AchTask($a,$this);
		}

		function getParentID() {
			return $this->parent_id;
		}

		function hasTieRace_open() {
			#return $this->tie_race;
			$iter = $this->child_open->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				if($curr->hasTieRace_open() && !$curr->inDev()) {
					return true;
				}
			}

			return false;
		}

		function hasTieAlign_open() {
			#return $this->tie_civ;
			$iter = $this->child_open->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				if($curr->hasTieAlign_open() && !$curr->inDev()) {
					return true;
				}
			}

			return false;
		}

		function hasTieRace_done() {
			#return $this->tie_race;
			$iter = $this->child_done->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				if($curr->hasTieRace_done() && !$curr->inDev()) {
					return true;
				}
			}

			return false;
		}

		function hasTieAlign_done() {
			#return $this->tie_civ;
			$iter = $this->child_done->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				if($curr->hasTieAlign_done() && !$curr->inDev()) {
					return true;
				}
			}

			return false;
		}

		function hasTieRaceDev() {
			#return $this->tie_race;
			$iter = $this->nodes->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				if($curr->hasTieRaceDev()) {
					return true;
				}
			}

			return false;
		}

		function hasTieAlignDev() {
			#return $this->tie_civ;
			$iter = $this->nodes->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				if($curr->hasTieAlignDev()) {
					return true;
				}
			}

			return false;
		}



		function isTiedRace_open($r) {
			#return $this->tie_race;
			$iter = $this->child_open->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				if($curr->isTiedRace_open($r)) {
					return true;
				}
			}

			return false;
		}

		function isTiedAlign_open($cult,$civ) {
			#return $this->tie_civ;
			$iter = $this->child_open->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				if($curr->isTiedAlign_open($cult,$civ)) {
					return true;
				}
			}

			return false;
		}

		function isTiedRace_done($r) {
			#return $this->tie_race;
			$iter = $this->child_done->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				if($curr->isTiedRace_done($r)) {
					return true;
				}
			}

			return false;
		}

		function isTiedAlign_done($cult,$civ) {
			#return $this->tie_civ;
			$iter = $this->child_done->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				if($curr->isTiedAlign_done($cult,$civ)) {
					return true;
				}
			}

			return false;
		}


		function getImage() {
			return $this->image;
		}

		function getName() {
			return $this->name;
		}

		function getValueDone() { // calculate the yubopoints that are already done
			$val = 0;
			$iter = $this->getDone();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				$val += $curr->getValue();
			}
			return $val;
		}

		function getValueOpen() { // get the yubopoints of the next open task
			$iter = $this->getOpen();
			if($iter->hasNext()) {
				$curr = $iter->getNext();
				return $curr->getValue();
			}
			return 0;
		}

		function fillTemplate($insert = array()) { // fill the naming template with given value
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

		function getSticky() {
			return $this->sticky;
		}

		function isSticky() {
			return ($this->sticky == 1);
		}

		function isHeroic() { // check parent category if it is heroic
			return $this->parent->isHeroic();
		}

		function isContest() {
			return $this->parent->isContest();
		}
		
	}
?>