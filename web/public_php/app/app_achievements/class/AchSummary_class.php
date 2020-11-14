<?php
	class AchSummary extends AchList implements Tieable {
		private $menu;
		private $stats;

		function AchSummary(&$menu,$size = 10) {
			global $DBc,$_USER;

			parent::__construct();

			$this->menu = $menu;

			#die("x:".$size);

			//read all recent tasks of user
			//make distinct achievement list

			$res = $DBc->sqlQuery("SELECT DISTINCT apt_date,aa_id,ach.*,(SELECT aal_name FROM ach_achievement_lang WHERE aal_lang='".$_USER->getLang()."' AND aal_achievement=ach.aa_id) as aal_name, (SELECT aal_template FROM ach_achievement_lang WHERE aal_lang='".$_USER->getLang()."' AND aal_achievement=ach.aa_id) as aal_template FROM ach_achievement as ach,ach_task,ach_player_task WHERE at_achievement=aa_id AND ach.aa_dev='0' AND apt_player='".$_USER->getID()."' AND apt_task=at_id ORDER by apt_date DESC LIMIT 0,".$size);

			#echo var_export($res,true);

			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$this->addDone($this->makeChild($res[$i]));
			}
		}
		
		#@override: Parentum::makeChild()
		protected function makeChild($a) {
			return new AchAchievement($a,$this);
		}

		function getSummary() {
			if(!is_array($this->stats)) { // only load if needed
				//now we have to find the # of tasks for each main menu entry
				//and also sum up how many have been completed
				$this->stats = array(); // [][name,done,total]

				$iter = $this->menu->getIterator();
				while($iter->hasNext()) {
					$curr = $iter->getNext();
					

					if($curr->getID() == 0 || $curr->inDev()) {
						continue; // skip summary page
					}

					$res = $this->sumStats($curr);
					$this->stats[] = array($curr->getName(),$res[0],$res[1],$res[2]);
				}
			}

			return $this->stats;
		}

		private function sumStats(&$node) {
			global $DBc,$_USER;

			#return array(0,0);

			#echo ">".gettype($node)."<";

			$done = 0;
			$total = 0;
			$hero = false;

			//read for current ID
			//sum
			$res = $DBc->sqlQuery("SELECT count(at_id) as anz FROM ach_task,ach_achievement,ach_player_task WHERE aa_category='".$node->getID()."' AND at_achievement=aa_id AND apt_player='".$_USER->getID()."' AND apt_task=at_id");
			$done += $res[0]["anz"];

			$res = $DBc->sqlQuery("SELECT count(at_id) as anz FROM ach_task,ach_achievement WHERE aa_category='".$node->getID()."' AND at_achievement=aa_id AND aa_dev='0' AND at_dev='0'");
			$total += $res[0]["anz"];

			$res = $DBc->sqlQuery("SELECT ac_heroic FROM ach_category WHERE ac_id='".$node->getID()."' AND ac_dev='0'");
			if($res[0]["ac_heroic"] == 1) {
				$hero = true;
			}
			
			$iter = $node->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();

				$res = $this->sumStats($curr);
				$done += $res[0];
				$total += $res[1];
				$hero = ($hero == true || $res[2] == true);
			}

			return array($done,$total,$hero);

		}

		function isTiedCult() {
			return false;
		}

		function isTiedCiv() {
			return false;
		}

		function getCurrentCiv() {
			return "c_neutral";
		}

		function getCurrentCult() {
			return "c_neutral";
		}

		function getCurrentRace() {
			return "r_matis";
		}

		function isHeroic() {
			return false;
		}

		function isContest() {
			return false;
		}

		function hasTieRace_open()
		{
			return false;
		}

		function hasTieAlign_open()
		{
			return false;
		}

		function hasTieRace_done()
		{
			return false;
		}

		function hasTieAlign_done()
		{
			return false;
		}

		function hasTieRaceDev()
		{
			return false;
		}

		function hasTieAlignDev()
		{
			return false;
		}

		function isTiedRace_open($r)
		{
			return true;
		}

		function isTiedAlign_open($cult, $civ)
		{
			return true;
		}

		function isTiedRace_done($r)
		{
			return true;
		}

		function isTiedAlign_done($cult, $civ)
		{
			return true;
		}
	}
?>