<?php
	class AchPerk extends RenderNodeIterator {
		private $id;
		private $parent;
		private $achievement;
		private $value;
		private $name;
		private $done;
		private $dev;

		function AchPerk(&$data,&$parent) {
			global $DBc,$_USER;

			$this->id = $data['ap_id'];
			$this->parent = $parent;
			$this->achievement = $data['ap_achievement'];
			$this->value = $data['ap_value'];
			$this->name = $data['apl_name'];
			$this->done = $data['app_date'];
			$this->dev = $data['ap_dev'];

			$res = $DBc->sqlQuery("SELECT * FROM ach_objective LEFT JOIN (ach_objective_lang) ON (aol_lang='".$_USER->getLang()."' AND aol_objective=ao_id) LEFT JOIN (ach_player_objective) ON (apo_objective=ao_id AND apo_player='".$_USER->getID()."') WHERE ao_perk='".$this->id."'");
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$this->nodes[] = new AchObjective($res[$i]);
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
			return $this->parent->getTemplate(explode(";",$this->name));
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

		function inDev() {
			return ($this->dev == 1);
		}
	}
?>