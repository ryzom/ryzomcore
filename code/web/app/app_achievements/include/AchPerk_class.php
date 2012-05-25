<?php
	class AchPerk extends RenderNodeIterator {
		private $id;
		private $parent;
		private $achievement;
		private $value;
		private $name;

		function AchPerk(&$data,$lang) {
			global $db;

			$this->id = $data['ap_id'];
			$this->parent = $data['ap_parent'];
			$this->achievement = $data['ap_achievement'];
			$this->value = $data['ap_value'];
			$this->name = $data['apl_name'];

			$res = $db->sqlQuery("SELECT * FROM ach_objective LEFT JOIN (ach_objective_lang) ON (aol_lang='".$lang."' AND aol_objective=ao_id) WHERE ao_perk='".$this->id."'");
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$this->nodes[] = new AchObjective($res[$i],$lang);
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
	}
?>