<?php
	class AchSummary extends AchList {
		private $menu;
		private $stats;

		function AchSummary(&$menu,$user,$size = 10,$lang = 'en') {
			global $db;

			$this->menu = $menu;

			//read all recent perks of user
			//make distinct achievement list

			$res = $db->sqlQuery("SELECT DISTINCT aa_id,ach.*,(SELECT aal_name FROM ach_achievement_lang WHERE aal_lang='".$lang."' AND aal_achievement=ach.aa_id) as aal_name FROM ach_achievement as ach,ach_perk,ach_player_perk WHERE ap_achievement=aa_id AND app_player='".$user."' AND app_perk=ap_id ORDER by app_date DESC LIMIT 0,".($size-1));

			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$tmp = new AchAchievement($res[$i],$lang,$user);

				$this->child_done[] = sizeof($this->nodes);
				$this->nodes[] = $tmp;
			}
		}

		function getSummary($lang,$user) {
			if(!is_array($this->stats)) { // only load if needed
				//now we have to find the # of perks for each main menu entry
				//and also sum up how many have been completed
				$this->stats = array(); // [][name,done,total]

				$tmp = $this->menu->getChildren();
				foreach($tmp as $elem) {
					if($elem->getID() == 0) {
						continue; // skip summary page
					}
					$res = $this->sumStats($elem,$user);
					$this->stats[] = array($elem->getName(),$res[0],$res[1]);
				}
			}

			return $this->stats;
		}

		private function sumStats(&$node,$user) {
			global $db;

			$done = 0;
			$total = 0;

			//read for current ID
			//sum
			$res = $db->sqlQuery("SELECT count(ap_id) as anz FROM ach_perk,ach_achievement,ach_player_perk WHERE aa_category='".$node->getID()."' AND ap_achievement=aa_id AND app_player='".$user."' AND app_perk=ap_id");
			$done += $res[0]["anz"];

			$res = $db->sqlQuery("SELECT count(ap_id) as anz FROM ach_perk,ach_achievement WHERE aa_category='".$node->getID()."' AND ap_achievement=aa_id");
			$total += $res[0]["anz"];
			
			$tmp = $node->getChildren();
			foreach($tmp as $elem) {
				$res = $this->sumStats($elem,$user);
				$done += $res[0];
				$total += $res[1];
			}

			return array($done,$total);

		}
	}
?>