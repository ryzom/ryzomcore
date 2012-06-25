<?php
	class AchPerk extends Parentum {
		use Node;

		protected $achievement;
		protected $value;
		protected $name;
		protected $done;
		protected $dev;

		function AchPerk($data,$parent) {
			global $DBc,$_USER;
			
			$this->setParent($parent);
			$this->setID($data['ap_id']);
			$this->achievement = $data['ap_achievement'];
			$this->value = $data['ap_value'];
			$this->name = $data['apl_name'];
			$this->done = $data['app_date'];
			$this->dev = $data['ap_dev'];

			$res = $DBc->sqlQuery("SELECT * FROM ach_objective LEFT JOIN (ach_objective_lang) ON (aol_lang='".$_USER->getLang()."' AND aol_objective=ao_id) LEFT JOIN (ach_player_objective) ON (apo_objective=ao_id AND apo_player='".$_USER->getID()."') LEFT JOIN (ach_achievement) ON (aa_id=ao_metalink) WHERE ao_perk='".$this->id."'");
			$sz = sizeof($res);
			for($i=0;$i<$sz;$i++) {
				$this->addChild($this->makeChild($res[$i]));
			}
		}

		protected function makeChild($a) {
			return new AchObjective($a,$this);
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
			$iter = $this->getIterator();
			while($iter->hasNext()) {
				$curr = $iter->getNext();
				if($curr->getDisplay() != "hidden") {
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