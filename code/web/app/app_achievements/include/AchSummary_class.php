<?php
	class AchSummary extends AchList {

		function AchSummary($size = 10,$lang = 'en') {
			global $db;

			$res = $db->sqlQuery("SELECT * FROM ach_achievement LEFT JOIN (ach_achievement_lang) ON (aal_lang='".$lang."' AND aal_achievement=aa_id) WHERE aa_category='".$this->id."' AND aa_parent IS NULL");
			#MISSING: or parent is done
			#MISSING: player's status on achievement
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$tmp = new AchAchievement($res[$i],$lang);
				if($tmp->hasDone()) {
					$this->child_done[] = sizeof($this->nodes);
					$this->nodes[] = $tmp;
				}
			}
		}
	}
?>