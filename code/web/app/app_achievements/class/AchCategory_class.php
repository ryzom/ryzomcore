<?php
	class AchCategory extends AchList {
		private $id = false;
		private $ties_cult;
		private $ties_civ;

		function AchCategory($id,$cult,$civ) {
			global $DBc,$_USER;

			$this->id = $id;

			$res = $DBc->sqlQuery("SELECT * FROM ach_achievement LEFT JOIN (ach_achievement_lang) ON (aal_lang='".$_USER->getLang()."' AND aal_achievement=aa_id) WHERE aa_category='".$this->id."' AND aa_parent IS NULL AND (aa_tie_race IS NULL OR aa_tie_race='".$_USER->getParam('race')."') AND (aa_tie_cult IS NULL OR aa_tie_cult='".$cult."') AND (aa_tie_civ IS NULL OR aa_tie_civ='".$civ."') ORDER by aal_name ASC");
			#MISSING: or parent is done
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				#echo "Y";
				$tmp = new AchAchievement($res[$i]);
				#echo var_export($tmp,true);
				if($tmp->hasOpen()) {
					$this->child_open[] = sizeof($this->nodes);
				}
				if($tmp->hasDone()) {
					$this->child_done[] = sizeof($this->nodes);
				}

				$this->nodes[] = $tmp;
			}

			$res = $DBc->sqlQuery("SELECT count(*) FROM ach_achievement WHERE aa_tie_cult IS NOT NULL");
			$this->ties_cult = $res[0]['anz'];

			$res = $DBc->sqlQuery("SELECT count(*) FROM ach_achievement WHERE aa_tie_civ IS NOT NULL");
			$this->ties_civ = $res[0]['anz'];
		}

		function getID() {
			return $this->id;
		}

		function isTiedCult() {
			return ($this->ties_cult > 0);
		}

		function isTiedCiv() {
			return ($this->ties_civ > 0);
		}
	}
?>