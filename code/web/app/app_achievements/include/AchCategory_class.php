<?php
	class AchCategory extends AchList {
		private $id = false;

		function AchCategory($id,$lang = 'en') {
			global $db;

			$this->id = $id;

			$res = $db->sqlQuery("SELECT * FROM ach_achievement LEFT JOIN (ach_achievement_lang) ON (aal_lang='".$lang."' AND aal_achievement=aa_id) WHERE aa_category='".$this->id."' AND aa_parent IS NULL");
			#MISSING: or parent is done
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$tmp = new AchAchievement($res[$i],$lang);
				if($tmp->hasOpen()) {
					$this->child_open[] = sizeof($this->nodes);
				}
				if($tmp->hasDone()) {
					$this->child_done[] = sizeof($this->nodes);
				}

				$this->nodes[] = $tmp;
			}
		}

		function getID() {
			return $this->id;
		}
	}
?>