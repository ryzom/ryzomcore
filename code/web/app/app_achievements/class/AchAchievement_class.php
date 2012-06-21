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
		private $template;
		private $dev;

		function AchAchievement(&$data) {
			global $DBc,$_USER;

			$this->id = $data['aa_id'];
			$this->parent = $data['aa_parent'];
			$this->category = $data['aa_category'];
			$this->tie_race = $data['aa_tie_race'];
			$this->tie_civ = $data['aa_tie_civ'];
			$this->tie_cult = $data['aa_tie_cult'];
			$this->image = $data['aa_image'];
			$this->name = $data['aal_name'];
			$this->template = $data['aal_template'];
			$this->dev = $data['aa_dev'];

			#echo $this->id;

			$res = $DBc->sqlQuery("SELECT * FROM ach_perk LEFT JOIN (ach_perk_lang) ON (apl_lang='".$_USER->getLang()."' AND apl_perk=ap_id) LEFT JOIN (ach_player_perk) ON (app_perk=ap_id AND app_player='".$_USER->getID()."') WHERE ap_achievement='".$this->id."' AND ap_parent IS NULL");
			#MISSING: or parent is done
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				#echo "Z";
				$res[$i]['this'] = $this;
				$tmp = $this->makeChild($res[$i]);

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

		function makeChild(&$a) {
			return new AchPerk($a);
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

		function getTemplate($insert = array()) {
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

		function inDev() {
			return ($this->dev == 1);
		}
	}
?>