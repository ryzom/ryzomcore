<?php
	class AchObjective extends Parentum {
		protected $task;
		protected $condition;
		protected $value;
		protected $name;
		protected $display;
		protected $done;
		protected $progress;
		protected $meta_image;
		protected $metalink;

		function AchObjective($data,$parent) {
			global $DBc,$_USER;

			parent::__construct();
			
			$this->setParent($parent);
			$this->setID($data['ao_id']);
			$this->task = $data['ao_task'];
			$this->condition = $data['ao_condition'];
			$this->value = $data['ao_value'];
			$this->name = $data['aol_name'];
			$this->display = $data['ao_display'];
			$this->done = $data['apo_date'];
			$this->meta_image = $data['aa_image'];
			$this->metalink = $data['ao_metalink'];

			if($this->metalink != null) {
				$this->name = $data['aal_name'];
			}

			$this->progress = $this->value;

			if(!$this->isDone()) {
				$res = $DBc->sqlQuery("SELECT sum(apa_value) as anz FROM ach_player_atom,ach_atom WHERE apa_atom=atom_id AND atom_objective='".$this->id."' AND apa_player='".$_USER->getID()."'");
				$this->progress = $res[0]['anz'];
			}
		}
		
		#@override: Parentum::makeChild()
		protected function makeChild($a) {
			return null;
		}

		function getMetaImage() {
			return $this->meta_image;
		}

		function getMetalink() {
			return $this->metalink;
		}

		function getTask() {
			return $this->task;
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

		function getDisplayName() {
			if(substr($this->name,0,1) == "!") {
				return substr($this->name,1);
			}
			return $this->parent->fillTemplate(explode(";",$this->name));
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