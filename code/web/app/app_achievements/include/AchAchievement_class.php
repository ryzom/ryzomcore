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

		function AchAchievement(&$data,$lang,$user) {
			global $db;

			$this->id = $data['aa_id'];
			$this->parent = $data['aa_parent'];
			$this->category = $data['aa_category'];
			$this->tie_race = $data['aa_tie_race'];
			$this->tie_civ = $data['aa_tie_civ'];
			$this->tie_cult = $data['aa_tie_cult'];
			$this->image = $data['aa_image'];
			$this->name = $data['aal_name'];
			$this->done = $data[''];

			#echo $this->id;

			$res = $db->sqlQuery("SELECT * FROM ach_perk LEFT JOIN (ach_perk_lang) ON (apl_lang='".$lang."' AND apl_perk=ap_id) LEFT JOIN (ach_player_perk) ON (app_perk=ap_id AND app_player='".$user."') WHERE ap_achievement='".$this->id."' AND ap_parent IS NULL");
			#MISSING: or parent is done
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				#echo "Z";
				$tmp = new AchPerk($res[$i],$lang,$user);

				#echo var_export($tmp,true);

				
				
				if($tmp->isDone()) {
					$this->child_done[] = sizeof($this->nodes);
				}
				else {
					$this->child_open[] = sizeof($this->nodes);
				}
				$this->nodes[] = $tmp;
				#MISSING: divide into groups -> open/done
			}

			#echo var_export($this->child_open,true);
			#echo "X-".$this->hasOpen();
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

		function getValueDone() {
			$val = 0;
			foreach($this->child_done as $elem) {
				$val += $this->nodes[$elem]->getValue();
			}
			return $val;
		}

		function getValueOpen() {
			return $this->nodes[$this->child_open[0]]->getValue();
		}
	}
?>