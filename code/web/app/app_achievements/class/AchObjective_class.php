<?php
	class AchObjective {
		private $id;
		private $perk;
		private $condition;
		private $value;
		private $name;
		private $display;
		private $done;
		private $progress;
		private $meta_image;

		function AchObjective(&$data) {
			global $DBc,$_USER;

			$this->id = $data['ao_id'];
			$this->perk = $data['ao_perk'];
			$this->condition = $data['ao_condition'];
			$this->value = $data['ao_value'];
			$this->name = $data['aol_name'];
			$this->display = $data['ao_display'];
			$this->done = $data['apo_date'];
			$this->meta_image = $data['aa_image'];

			$this->progress = $this->value;

			if(!$this->isDone()) {
				$res = $DBc->sqlQuery("SELECT count(*) as anz FROM ach_player_atom,ach_atom WHERE apa_atom=atom_id AND atom_objective='".$this->id."' AND apa_player='".$_USER->getId()."'");
				$this->progress = $res[0]['anz'];
			}
		}

		function getMetaImage() {
			return $this->meta_image;
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

		function getProgress() {
			return $this->progress;
		}

		function getName() {
			return $this->name;
		}

		function getDisplay() {
			return $this->display;
		}

		function isDone() {
			return ($this->done > 0);
		}

		function getDone() {
			return $this->done;
		}
	}
?>