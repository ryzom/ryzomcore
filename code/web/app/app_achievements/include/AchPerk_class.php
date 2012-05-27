<?php
	class AchPerk extends RenderNodeIterator {
		private $id;
		private $parent;
		private $achievement;
		private $value;
		private $name;
		private $done;

		function AchPerk(&$data,$lang,$user) {
			global $db;

			$this->id = $data['ap_id'];
			$this->parent = $data['ap_parent'];
			$this->achievement = $data['ap_achievement'];
			$this->value = $data['ap_value'];
			$this->name = $data['apl_name'];
			$this->done = $data['app_date'];

			$res = $db->sqlQuery("SELECT * FROM ach_objective LEFT JOIN (ach_objective_lang) ON (aol_lang='".$lang."' AND aol_objective=ao_id) LEFT JOIN (ach_player_objective) ON (apo_objective=ao_id AND apo_player='".$user."') WHERE ao_perk='".$this->id."'");
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$this->nodes[] = new AchObjective($res[$i],$lang,$user);
			}
		}

		function getID() {
			return $this->id;
		}

		function getParent() {
			return $this->parent;
		}

		function getAchievement() {
			return $this->achievement;
		}

		function getValue() {
			return $this->value;
		}

		function getName() {
			return $this->name;
		}

		function objDrawable() {
			foreach($this->nodes as $elem) {
				if($elem->getDisplay() != "hidden") {
					return true;
				}
			}
			return false;
		}

		function isDone() {
			return ($this->done > 0);
		}

		function getDone() {
			return $this->done;
		}
	}
?>