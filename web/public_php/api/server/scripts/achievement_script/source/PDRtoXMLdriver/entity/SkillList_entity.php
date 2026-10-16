<?php
	class SkillList extends Entity {
		public $skills;

		function SkillList() {
			$this->setName("skilllist");
			$this->skills = array();
		}
	}
?>