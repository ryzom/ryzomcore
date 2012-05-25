<?php
	class AchAchievement extends AchList {
		private $id;
		private $parent;
		private $category;
		private $tie_race;
		private $tie_civ;
		private $tie_cult;
		private $image;
		private $name;

		function AchAchievement(&$data,$lang) {
			global $db;

			$this->id = $data['aa_id'];
			$this->parent = $data['aa_parent'];
			$this->category = $data['aa_category'];
			$this->tie_race = $data['aa_tie_race'];
			$this->tie_civ = $data['aa_tie_civ'];
			$this->tie_cult = $data['aa_tie_cult'];
			$this->image = $data['aa_image'];
			$this->name = $data['aal_name'];

			$res = $db->sqlQuery("SELECT * FROM ach_perk LEFT JOIN (ach_perk_lang) ON (apl_lang='".$lang."' AND apl_achievement=ap_id) WHERE ap_achievement='".$this->id."' AND ap_parent IS NULL");
			#MISSING: or parent is done
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$tmp = new AchPerk($res[$i],$lang);

				$this->child_open[] = sizeof($this->nodes);
				$this->nodes[] = $tmp;
				/*if($res[$i]['']) {

				}
				else {

				}*/
				#MISSING: divide into groups -> open/done
			}
		}

		function getID() {
			return $this->id;
		}

		function getParent() {
			return $this->parent;
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

		function getValue() {
			$val = 0;
			foreach($this->child_done as $elem) {
				$val += $this->nodes[$elem]->getValue();
			}
			return $val;
		}
	}
?>