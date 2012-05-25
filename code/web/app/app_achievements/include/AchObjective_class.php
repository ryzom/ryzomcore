<?php
	class AchObjective {
		private $id;
		private $perk;
		private $condition;
		private $value;
		private $name;
		private $display;

		function AchObjective(&$data,$lang) {
			$this->id = $data['ao_id'];
			$this->perk = $data['ao_perk'];
			$this->condition = $data['ao_condition'];
			$this->value = $data['ao_value'];
			$this->name = $data['aol_name'];
			$this->display = $data['ao_display'];
		}

		function getID() {
			return $this->id;
		}

		function getPerk() {
			return $this->perk;
		}

		function getCondition() {
			return $this->condition;
		}

		function getValue() {
			return $this->value;
		}

		function getName() {
			return $this->name;
		}

		function getDisplay() {
			return $this->display;
		}
	}
?>