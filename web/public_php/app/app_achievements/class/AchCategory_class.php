<?php
	/*
	 * Category class that is loading all achievements tied to it.
	 */

	class AchCategory extends AchList implements Tieable {
		protected $ties_align_done;
		protected $ties_race_done;
		protected $ties_align_open;
		protected $ties_race_open;
		protected $ties_race_dev;
		protected $ties_align_dev;
		protected $cult;
		protected $civ;
		protected $race;
		protected $heroic;
		protected $contest;
		protected $allow_civ;
		protected $allow_cult;

		function AchCategory($id,$race = null,$cult = "c_neutral",$civ = "c_neutral") {
			global $DBc,$_USER;

			parent::__construct();

			if($civ != "%") {
				$civ = $DBc->sqlEscape($civ);
			}
			if($cult != "%") {
				$cult = $DBc->sqlEscape($cult);
			}
			$race = $DBc->sqlEscape($race);

			/*if($race == null) {
				$race = $_USER->getRace();
			}

			if($cult == null) {
				$cult = $_USER->getCult();
			}

			if($civ == null) {
				$civ = $_USER->getCiv();
			}*/

			$this->cult = $cult;
			$this->civ = $civ;
			$this->rave = $race;

			$this->id = $DBc->sqlEscape($id);

			$res = $DBc->sqlQuery("SELECT * FROM ach_achievement LEFT JOIN (ach_achievement_lang) ON (aal_lang='".$_USER->getLang()."' AND aal_achievement=aa_id) WHERE aa_category='".$this->id."' ORDER by aa_sticky DESC, aal_name ASC");

			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$tmp = $this->makeChild($res[$i]);
				
				if($tmp->hasOpen()) {
					$this->addOpen($tmp); #AchList::addOpen()
				}
				if($tmp->hasDone()) {
					$this->addDone($tmp); #AchList::addDone()
				}
			}

			$res = $DBc->sqlQuery("SELECT ac_heroic,ac_contest,ac_allow_civ,ac_allow_cult FROM ach_category WHERE ac_id='".$this->id."'");
			$this->heroic = $res[0]['ac_heroic'];
			$this->contest = $res[0]['ac_contest'];
			$this->allow_civ = $res[0]['ac_allow_civ'];
			$this->allow_cult = $res[0]['ac_allow_cult'];


			$iter = $this->nodes->getIterator();
			$tmp = false;
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				if($curr->hasTieRace_open() && !$curr->inDev()) {
					$tmp = true;
					break;
				}
			}
			$this->ties_race_open = $tmp;

			$iter = $this->nodes->getIterator();
			$tmp = false;
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				if($curr->hasTieAlign_open() && !$curr->inDev()) {
					$tmp = true;
					break;
				}
			}
			$this->ties_align_open = $tmp;

			$iter = $this->nodes->getIterator();
			$tmp = false;
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				if($curr->hasTieRace_done() && !$curr->inDev()) {
					$tmp = true;
					break;
				}
			}
			$this->ties_race_done = $tmp;

			$iter = $this->nodes->getIterator();
			$tmp = false;
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				if($curr->hasTieAlign_done() && !$curr->inDev()) {
					$tmp = true;
					break;
				}
			}
			$this->ties_align_done = $tmp;

			$iter = $this->nodes->getIterator();
			$tmp = false;
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				if($curr->hasTieRaceDev()) {
					$tmp = true;
					break;
				}
			}
			$this->ties_race_dev = $tmp;

			$iter = $this->nodes->getIterator();
			$tmp = false;
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				if($curr->hasTieAlignDev()) {
					$tmp = true;
					break;
				}
			}
			$this->ties_align_dev = $tmp;
		}
		
		#@override Parentum::makeChild()
		protected function makeChild($a) {
			return new AchAchievement($a,$this);
		}

		function isAllowedCult() {
			return ($this->allow_cult == 1);
		}

		function isAllowedCiv() {
			return ($this->allow_civ == 1);
		}

		function hasTieRace_open() {
			return $this->ties_race_open;
		}

		function hasTieAlign_open() {
			return $this->ties_align_open;
		}

		function hasTieRace_done() {
			return $this->ties_race_done;
		}

		function hasTieAlign_done() {
			return $this->ties_align_done;
		}

		function hasTieRaceDev() {
			return $this->ties_race_dev;
		}

		function hasTieAlignDev() {
			return $this->ties_align_dev;
		}

		function getCurrentCiv() {
			return $this->civ;
		}

		function getCurrentCult() {
			return $this->cult;
		}

		function getCurrentRace() {
			return $this->race;
		}

		function isTiedRace_open($r) {
			return null;
		}

		function isTiedAlign_open($cult,$civ) {
			return null;
		}

		function isTiedRace_done($r) {
			return null;
		}

		function isTiedAlign_done($cult,$civ) {
			return null;
		}

		function isHeroic() {
			return ($this->heroic == 1);
		}

		function isContest() {
			return ($this->contest == 1);
		}
	}
?>